/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/config_concatenation.h"
#include "configcpp/detail/simple_config_list.h"
#include "configcpp/detail/simple_config_origin.h"
#include "configcpp/detail/config_string.h"
#include "configcpp/detail/resolve_context.h"
#include "configcpp/config_object.h"
#include "configcpp/config_exception.h"

namespace config {

ConfigConcatenation::ConfigConcatenation(const ConfigOriginPtr& origin, const VectorAbstractConfigValue& pieces) :
    AbstractConfigValue(origin),
    pieces(pieces) {
    if (pieces.size() < 2) {
        throw ConfigExceptionBugOrBroken("Created concatenation with less than 2 items: " + toString());
    }

    bool hadUnmergeable = false;
    for (auto& p : pieces) {
        if (instanceof<ConfigConcatenation>(p)) {
            throw ConfigExceptionBugOrBroken("ConfigConcatenation should never be nested: " + toString());
        }
        if (instanceof<Unmergeable>(p)) {
            hadUnmergeable = true;
        }
    }
    if (!hadUnmergeable) {
        throw ConfigExceptionBugOrBroken("Created concatenation without an unmergeable in it: " + toString());
    }
}

ConfigExceptionNotResolved ConfigConcatenation::notResolved() {
    return ConfigExceptionNotResolved("need to Config::resolve(), see the API docs for Config::resolve(); substitution not resolved: " + toString());
}

ConfigValueType ConfigConcatenation::valueType() {
    throw notResolved();
}

ConfigVariant ConfigConcatenation::unwrapped() {
    throw notResolved();
}

AbstractConfigValuePtr ConfigConcatenation::newCopy(const ConfigOriginPtr& newOrigin) {
    return make_instance(newOrigin, pieces);
}

bool ConfigConcatenation::ignoresFallbacks() {
    // we can never ignore fallbacks because if a child ConfigReference
    // is self-referential we have to look lower in the merge stack
    // for its value.
    return false;
}

VectorAbstractConfigValue ConfigConcatenation::unmergedValues() {
    return {shared_from_this()};
}

void ConfigConcatenation::join(VectorAbstractConfigValue& builder, const AbstractConfigValuePtr& right) {
    auto left = builder.back();
    // Since this depends on the type of two instances, I couldn't think
    // of much alternative to an instanceof chain. Visitors are sometimes
    // used for multiple dispatch but seems like overkill.
    AbstractConfigValuePtr joined;
    if (instanceof<ConfigObject>(left) && instanceof<ConfigObject>(right)) {
        joined = std::dynamic_pointer_cast<AbstractConfigValue>(right->withFallback(left));
    }
    else if (instanceof<SimpleConfigList>(left) && instanceof<SimpleConfigList>(right)) {
        joined = std::static_pointer_cast<SimpleConfigList>(left)->concatenate(std::static_pointer_cast<SimpleConfigList>(right));
    }
    else if (instanceof<ConfigConcatenation>(left) || instanceof<ConfigConcatenation>(right)) {
        throw ConfigExceptionBugOrBroken("unflattened ConfigConcatenation");
    }
    else if (instanceof<Unmergeable>(left) || instanceof<Unmergeable>(right)) {
        // leave joined=null, cannot join
    }
    else {
        // handle primitive type or primitive type mixed with object or list
        std::string s1 = left->transformToString();
        std::string s2 = right->transformToString();
        if (s1.empty() || s2.empty()) {
            throw ConfigExceptionWrongType(left->origin(),
                    "Cannot concatenate object or list with a non-object-or-list, " +
                    left->toString() + " and " + right->toString() + " are not compatible");
        }
        else {
            auto joinedOrigin = SimpleConfigOrigin::mergeOrigins(left->origin(), right->origin());
            joined = ConfigString::make_instance(joinedOrigin, s1 + s2);
        }
    }

    if (!joined) {
        builder.push_back(right);
    }
    else {
        builder.resize(builder.size() - 1);
        builder.push_back(joined);
    }
}

VectorAbstractConfigValue ConfigConcatenation::consolidate(const VectorAbstractConfigValue& pieces) {
    if (pieces.size() < 2) {
        return pieces;
    }
    else {
        VectorAbstractConfigValue flattened;
        flattened.reserve(pieces.size());
        for (auto& v : pieces) {
            if (instanceof<ConfigConcatenation>(v)) {
                flattened.insert(flattened.end(), std::static_pointer_cast<ConfigConcatenation>(v)->pieces.begin(), std::static_pointer_cast<ConfigConcatenation>(v)->pieces.end());
            }
            else {
                flattened.push_back(v);
            }
        }

        VectorAbstractConfigValue consolidated;
        consolidated.reserve(flattened.size());
        for (auto& v : flattened) {
            if (consolidated.empty()) {
                consolidated.push_back(v);
            }
            else {
                join(consolidated, v);
            }
        }

        return consolidated;
    }
}

AbstractConfigValuePtr ConfigConcatenation::concatenate(const VectorAbstractConfigValue& pieces) {
    VectorAbstractConfigValue consolidated = consolidate(pieces);
    if (consolidated.empty()) {
        return nullptr;
    }
    else if (consolidated.size() == 1) {
        return consolidated.front();
    }
    else {
        auto mergedOrigin = SimpleConfigOrigin::mergeOrigins(consolidated);
        return make_instance(mergedOrigin, consolidated);
    }
}

AbstractConfigValuePtr ConfigConcatenation::resolveSubstitutions(const ResolveContextPtr& context) {
    VectorAbstractConfigValue resolved;
    resolved.reserve(pieces.size());
    for (auto& p : pieces) {
        // to concat into a string we have to do a full resolve,
        // so unrestrict the context
        auto r = context->unrestricted()->resolve(p);
        if (!r) {
            // it was optional... omit
        }
        else {
            resolved.push_back(r);
        }
    }

    // now need to concat everything
    VectorAbstractConfigValue joined = consolidate(resolved);
    if (joined.size() != 1) {
        throw ConfigExceptionBugOrBroken("Resolved list should always join to exactly one value, not " + boost::lexical_cast<std::string>(joined.size()));
    }
    return joined.front();
}

ResolveStatus ConfigConcatenation::resolveStatus() {
    return ResolveStatus::UNRESOLVED;
}

AbstractConfigValuePtr ConfigConcatenation::relativized(const PathPtr& prefix) {
    VectorAbstractConfigValue newPieces;
    newPieces.reserve(pieces.size());
    for (auto& p : pieces) {
        newPieces.push_back(p->relativized(prefix));
    }
    return make_instance(origin(), newPieces);
}

bool ConfigConcatenation::canEqual(const ConfigVariant& other) {
    return instanceof<ConfigConcatenation>(other);
}

bool ConfigConcatenation::equals(const ConfigVariant& other) {
    // note that "origin" is deliberately NOT part of equality
    if (instanceof<ConfigConcatenation>(other)) {
        return canEqual(other) &&
                this->pieces.size() == dynamic_get<ConfigConcatenation>(other)->pieces.size() &&
                std::equal(this->pieces.begin(), this->pieces.end(), dynamic_get<ConfigConcatenation>(other)->pieces.begin(), configEquals<AbstractConfigValuePtr>());
    }
    else {
        return false;
    }
}

uint32_t ConfigConcatenation::hashCode() {
    // note that "origin" is deliberately NOT part of equality
    size_t hash = 0;
    for (auto& v : pieces) {
        boost::hash_combine(hash, v->hashCode());
    }
    return static_cast<uint32_t>(hash);
}

void ConfigConcatenation::render(std::string& s, uint32_t indent, const ConfigRenderOptionsPtr& options) {
    for (auto& p : pieces) {
        p->render(s, indent, options);
    }
}

}
