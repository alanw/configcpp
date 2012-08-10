/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/simple_config_object.h"
#include "configcpp/detail/simple_config_origin.h"
#include "configcpp/detail/resolve_status.h"
#include "configcpp/detail/resolve_context.h"
#include "configcpp/detail/path.h"
#include "configcpp/config_render_options.h"
#include "configcpp/config_origin.h"

namespace config {

SimpleConfigObject::SimpleConfigObject(const ConfigOriginPtr& origin, const MapAbstractConfigValue& value, ResolveStatus status, bool ignoresFallbacks) :
    AbstractConfigObject(origin),
    value(value.begin(), value.end()),
    ignoresFallbacks_(ignoresFallbacks) {
    resolved = (status == ResolveStatus::RESOLVED);

    // Kind of an expensive debug check. Comment out?
    if (status != ResolveStatusEnum::fromValues(value)) {
        throw ConfigExceptionBugOrBroken("SimpleConfigObject created with wrong resolve status");
    }
}

SimpleConfigObject::SimpleConfigObject(const ConfigOriginPtr& origin, const MapAbstractConfigValue& value) :
    SimpleConfigObject(origin, value, ResolveStatusEnum::fromValues(value), false) {
}

ConfigObjectPtr SimpleConfigObject::withOnlyKey(const std::string& key) {
    return withOnlyPath(Path::newKey(key));
}

ConfigObjectPtr SimpleConfigObject::withoutKey(const std::string& key) {
    return withoutPath(Path::newKey(key));
}

AbstractConfigObjectPtr SimpleConfigObject::withOnlyPathOrNull(const PathPtr& path) {
    std::string key = path->first();
    auto next = path->remainder();
    auto val = value.find(key);
    auto v = val == value.end() ? nullptr : std::dynamic_pointer_cast<AbstractConfigValue>(val->second);

    if (next) {
        if (v && instanceof<AbstractConfigObject>(v)) {
            v = std::static_pointer_cast<AbstractConfigObject>(v)->withOnlyPathOrNull(next);
        }
        else {
            // if the path has more elements but we don't have an object,
            // then the rest of the path does not exist.
            v = nullptr;
        }
    }

    if (!v) {
        return nullptr;
    }
    else {
        return SimpleConfigObject::make_instance(origin(), MapAbstractConfigValue({{key, v}}), v->resolveStatus(), ignoresFallbacks_);
    }
}

AbstractConfigObjectPtr SimpleConfigObject::withOnlyPath(const PathPtr& path) {
    auto o = std::static_pointer_cast<SimpleConfigObject>(withOnlyPathOrNull(path));
    if (!o) {
        return SimpleConfigObject::make_instance(origin(), MapAbstractConfigValue(), ResolveStatus::RESOLVED, ignoresFallbacks_);
    }
    else {
        return o;
    }
}

AbstractConfigObjectPtr SimpleConfigObject::withoutPath(const PathPtr& path) {
    std::string key = path->first();
    auto next = path->remainder();
    auto val = value.find(key);
    auto v = val == value.end() ? nullptr : std::dynamic_pointer_cast<AbstractConfigValue>(val->second);

    if (v && next && instanceof<AbstractConfigObject>(v)) {
        v = std::static_pointer_cast<AbstractConfigObject>(v)->withoutPath(next);
        MapAbstractConfigValue updated = MiscUtils::dynamic_map<MapAbstractConfigValue>(value);
        updated[key] = v;
        return SimpleConfigObject::make_instance(origin(), updated, ResolveStatusEnum::fromValues(updated), ignoresFallbacks_);
    }
    else if (next || !v) {
        // can't descend, nothing to remove
        return shared_from_this();
    }
    else {
        MapAbstractConfigValue smaller;
        for (auto& old : value) {
            if (old.first != key) {
                smaller[old.first] = std::dynamic_pointer_cast<AbstractConfigValue>(old.second);
            }
        }
        return SimpleConfigObject::make_instance(origin(), smaller, ResolveStatusEnum::fromValues(smaller), ignoresFallbacks_);
    }
}

AbstractConfigValuePtr SimpleConfigObject::attemptPeekWithPartialResolve(const std::string& key) {
    auto val = value.find(key);
    return val == value.end() ? nullptr : std::dynamic_pointer_cast<AbstractConfigValue>(val->second);
}

AbstractConfigObjectPtr SimpleConfigObject::newCopy(ResolveStatus newStatus, const ConfigOriginPtr& newOrigin, bool newIgnoresFallbacks) {
    return SimpleConfigObject::make_instance(newOrigin, MiscUtils::dynamic_map<MapAbstractConfigValue>(value), newStatus, newIgnoresFallbacks);
}

AbstractConfigObjectPtr SimpleConfigObject::newCopy(ResolveStatus newStatus, const ConfigOriginPtr& newOrigin) {
    return newCopy(newStatus, newOrigin, ignoresFallbacks_);
}

AbstractConfigValuePtr SimpleConfigObject::withFallbacksIgnored() {
    if (ignoresFallbacks_) {
        return shared_from_this();
    }
    else {
        return newCopy(resolveStatus(), origin(), true);
    }
}

ResolveStatus SimpleConfigObject::resolveStatus() {
    return ResolveStatusEnum::fromBool(resolved);
}

bool SimpleConfigObject::ignoresFallbacks() {
    return ignoresFallbacks_;
}

ConfigVariant SimpleConfigObject::unwrapped() {
    MapVariant m;
    for (auto& e : value) {
        m[e.first] = e.second->unwrapped();
    }
    return m;
}

AbstractConfigValuePtr SimpleConfigObject::mergedWithObject(const AbstractConfigObjectPtr& abstractFallback) {
    requireNotIgnoringFallbacks();

    if (!instanceof<SimpleConfigObject>(abstractFallback)) {
        throw ConfigExceptionBugOrBroken("should not be reached (merging non-SimpleConfigObject)");
    }

    auto fallback = std::static_pointer_cast<SimpleConfigObject>(abstractFallback);

    bool changed = false;
    bool allResolved = true;
    MapAbstractConfigValue merged;
    SetString allKeys;

    for (auto& v : value) {
        allKeys.insert(v.first);
    }
    for (auto& v : fallback->value) {
        allKeys.insert(v.first);
    }

    for (auto& key : allKeys) {
        auto firstVal = value.find(key);
        auto first = firstVal == value.end() ? nullptr : std::dynamic_pointer_cast<AbstractConfigValue>(firstVal->second);

        auto secondVal = fallback->value.find(key);
        auto second = secondVal == fallback->value.end() ? nullptr : std::dynamic_pointer_cast<AbstractConfigValue>(secondVal->second);

        AbstractConfigValuePtr kept;
        if (!first) {
            kept = second;
        }
        else if (!second) {
            kept = first;
        }
        else {
            kept = std::dynamic_pointer_cast<AbstractConfigValue>(first->withFallback(second));
        }

        merged[key] = kept;

        if (first != kept) {
            changed = true;
        }

        if (kept->resolveStatus() == ResolveStatus::UNRESOLVED) {
            allResolved = false;
        }
    }

    ResolveStatus newResolveStatus = ResolveStatusEnum::fromBool(allResolved);
    bool newIgnoresFallbacks = fallback->ignoresFallbacks();

    if (changed) {
        return SimpleConfigObject::make_instance(mergeOrigins(VectorAbstractConfigObject({shared_from_this(), fallback})), merged, newResolveStatus, newIgnoresFallbacks);
    }
    else if (newResolveStatus != resolveStatus() || newIgnoresFallbacks != ignoresFallbacks()) {
        return newCopy(newResolveStatus, origin(), newIgnoresFallbacks);
    }
    else {
        return shared_from_this();
    }
}

SimpleConfigObjectPtr SimpleConfigObject::modify(const NoExceptionsModifierPtr& modifier) {
    try {
        return modifyMayThrow(modifier);
    }
    catch (ConfigException&) {
        throw;
    }
    catch (std::exception& e) {
        throw ConfigExceptionBugOrBroken(std::string("unexpected checked exception:") + e.what());
    }
}

SimpleConfigObjectPtr SimpleConfigObject::modifyMayThrow(const ModifierPtr& modifier) {
    boost::optional<MapAbstractConfigValue> changes;

    for (auto& kv : value) {
        // "modified" may be null, which means remove the child;
        // to do that we put null in the "changes" map.
        auto modified = modifier->modifyChildMayThrow(kv.first, std::dynamic_pointer_cast<AbstractConfigValue>(kv.second));
        if (modified != kv.second) {
            if (!changes) {
                changes = MapAbstractConfigValue();
            }
            (*changes)[kv.first] = modified;
        }
    }

    if (!changes) {
        return shared_from_this();
    }
    else {
        MapAbstractConfigValue modified;
        bool sawUnresolved = false;
        for (auto& kv : value) {
            if (changes->count(kv.first) > 0) {
                auto newValue = changes->find(kv.first);
                if (newValue != changes->end() && newValue->second) {
                    modified[kv.first] = newValue->second;
                    if (newValue->second->resolveStatus() == ResolveStatus::UNRESOLVED) {
                        sawUnresolved = true;
                    }
                }
                else {
                    // remove this child; don't put it in the new map.
                }
            }
            else {
                auto newValue = value.find(kv.first);
                modified[kv.first] = std::dynamic_pointer_cast<AbstractConfigValue>(newValue->second);
                if (std::dynamic_pointer_cast<AbstractConfigValue>(newValue->second)->resolveStatus() == ResolveStatus::UNRESOLVED) {
                    sawUnresolved = true;
                }
            }
        }
        return SimpleConfigObject::make_instance(origin(), modified, sawUnresolved ? ResolveStatus::UNRESOLVED : ResolveStatus::RESOLVED, ignoresFallbacks());
    }
}

AbstractConfigValuePtr SimpleConfigObject::resolveSubstitutions(const ResolveContextPtr& context) {
    if (resolveStatus() == ResolveStatus::RESOLVED) {
        return shared_from_this();
    }

    try {
        return modifyMayThrow(SimpleConfigObjectModifier::make_instance(context));
    }
    catch (NotPossibleToResolve&) {
        throw;
    }
    catch (ConfigException&) {
        throw;
    }
    catch (std::exception& e) {
        throw ConfigExceptionBugOrBroken(std::string("unexpected checked exception:") + e.what());
    }
}

SimpleConfigObjectModifier::SimpleConfigObjectModifier(const ResolveContextPtr& context) :
    context(context) {
}

AbstractConfigValuePtr SimpleConfigObjectModifier::modifyChildMayThrow(const std::string& key, const AbstractConfigValuePtr& v) {
    if (context->isRestrictedToChild()) {
        if (key == context->restrictToChild()->first()) {
            auto remainder = context->restrictToChild()->remainder();
            if (remainder) {
                return context->restrict(remainder)->resolve(v);
            }
            else {
                // we don't want to resolve the leaf child.
                return v;
            }
        }
        else {
            // not in the restrictToChild path
            return v;
        }
    }
    else {
        // no restrictToChild, resolve everything
        return context->unrestricted()->resolve(v);
    }
}

AbstractConfigValuePtr SimpleConfigObject::relativized(const PathPtr& prefix) {
    return modify(SimpleConfigObjectNoExceptionsModifier::make_instance(prefix));
}

SimpleConfigObjectNoExceptionsModifier::SimpleConfigObjectNoExceptionsModifier(const PathPtr& prefix) :
    prefix(prefix) {
}

AbstractConfigValuePtr SimpleConfigObjectNoExceptionsModifier::modifyChild(const std::string& key, const AbstractConfigValuePtr& v) {
    return v->relativized(prefix);
}

void SimpleConfigObject::render(std::string& s, uint32_t indent_, const ConfigRenderOptionsPtr& options) {
    if (value.empty()) {
        s += "{}";
    }
    else {
        s += "{";
        if (options->getFormatted()) {
            s += "\n";
        }
        for (auto& kv : value) {
            if (options->getOriginComments()) {
                indent(s, indent_ + 1, options);
                s += "# ";
                s += kv.second->origin()->description();
                s += "\n";
            }
            if (options->getComments()) {
                for (auto& comment : kv.second->origin()->comments()) {
                    indent(s, indent_ + 1, options);
                    s += "# ";
                    s += comment;
                    s += "\n";
                }
            }
            indent(s, indent_ + 1, options);
            std::dynamic_pointer_cast<AbstractConfigValue>(kv.second)->render(s, indent_ + 1, kv.first, options);
            s += ",";
            if (options->getFormatted()) {
                s += "\n";
            }
        }
        // chop comma or newline
        s.resize(s.length() - 1);
        if (options->getFormatted()) {
            s.resize(s.length() - 1); // also chop comma
            s += "\n"; // put a newline back
            indent(s, indent_, options);
        }
        s += "}";
    }
}

ConfigValuePtr SimpleConfigObject::get(const std::string& key) {
    auto v = value.find(key);
    return v == value.end() ? nullptr : v->second;
}

bool SimpleConfigObject::mapEquals(const MapConfigValue& a, const MapConfigValue& b) {
    return MiscUtils::map_equals(a, b);
}

uint32_t SimpleConfigObject::mapHash(const MapConfigValue& m) {
    // the keys have to be sorted, otherwise we could be equal
    // to another map but have a different hashcode.
    std::set<std::string> keys;
    MiscUtils::key_set(m.begin(), m.end(), std::inserter(keys, keys.end()));

    uint32_t valuesHash = 0;
    size_t keysHash = 0;
    for (auto& k : keys) {
        valuesHash += std::dynamic_pointer_cast<AbstractConfigValue>(m.find(k)->second)->hashCode();
        boost::hash_combine(keysHash, std::hash<std::string>()(k));
    }
    return 41 * (41 + static_cast<uint32_t>(keysHash)) + valuesHash;
}

bool SimpleConfigObject::canEqual(const ConfigVariant& other) {
    return instanceof<SimpleConfigObject>(other);
}

bool SimpleConfigObject::equals(const ConfigVariant& other) {
    // note that "origin" is deliberately NOT part of equality
    // neither are other "extras" like ignoresFallbacks or resolve status.
    if (instanceof<SimpleConfigObject>(other)) {
        return canEqual(other) && mapEquals(value, dynamic_get<SimpleConfigObject>(other)->value);
    }
    else {
        return false;
    }
}

uint32_t SimpleConfigObject::hashCode() {
    // note that "origin" is deliberately NOT part of equality
    // neither are other "extras" like ignoresFallbacks or resolve status.
    return mapHash(value);
}

MapConfigValue::const_iterator SimpleConfigObject::begin() const {
    return value.begin();
}

MapConfigValue::const_iterator SimpleConfigObject::end() const {
    return value.end();
}

MapConfigValue::mapped_type SimpleConfigObject::operator[](const MapConfigValue::key_type& key) const {
    auto val = value.find(key);
    return val == value.end() ? nullptr : val->second;
}

bool SimpleConfigObject::empty() const {
    return value.empty();
}

MapConfigValue::size_type SimpleConfigObject::size() const {
    return value.size();
}

MapConfigValue::size_type SimpleConfigObject::count(const MapConfigValue::key_type& key) const {
    return value.count(key);
}

MapConfigValue::const_iterator SimpleConfigObject::find(const MapConfigValue::key_type& key) const {
    return value.find(key);
}

SimpleConfigObjectPtr SimpleConfigObject::makeEmpty() {
    static std::string EMPTY_NAME = "empty config";
    static auto emptyInstance = makeEmpty(SimpleConfigOrigin::newSimple(EMPTY_NAME));
    return emptyInstance;
}

SimpleConfigObjectPtr SimpleConfigObject::makeEmpty(const ConfigOriginPtr& origin) {
    if (!origin) {
        return makeEmpty();
    }
    else {
        return SimpleConfigObject::make_instance(origin, MapAbstractConfigValue());
    }
}

SimpleConfigObjectPtr SimpleConfigObject::makeEmptyMissing(const ConfigOriginPtr& baseOrigin) {
    return SimpleConfigObject::make_instance(SimpleConfigOrigin::newSimple(baseOrigin->description() + " (not found)"), MapAbstractConfigValue());
}

}
