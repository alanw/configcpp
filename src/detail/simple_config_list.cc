/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/simple_config_list.h"
#include "configcpp/detail/simple_config_origin.h"
#include "configcpp/detail/resolve_status.h"
#include "configcpp/detail/resolve_context.h"
#include "configcpp/config_value_type.h"
#include "configcpp/config_render_options.h"
#include "configcpp/config_origin.h"

namespace config {

SimpleConfigList::SimpleConfigList(const ConfigOriginPtr& origin, const VectorAbstractConfigValue& value) :
    SimpleConfigList(origin, value, ResolveStatusEnum::fromValues(value)) {
}

SimpleConfigList::SimpleConfigList(const ConfigOriginPtr& origin, const VectorAbstractConfigValue& value, ResolveStatus status) :
    AbstractConfigValue(origin),
    value(value.begin(), value.end()) {
    resolved = (status == ResolveStatus::RESOLVED);
    // kind of an expensive debug check (makes this constructor pointless)
    if (status != ResolveStatusEnum::fromValues(value)) {
        throw ConfigExceptionBugOrBroken("SimpleConfigList created with wrong resolve status");
    }
}

ConfigValueType SimpleConfigList::valueType() {
    return ConfigValueType::LIST;
}

ConfigVariant SimpleConfigList::unwrapped() {
    VectorVariant list;
    list.reserve(value.size());
    for (auto& v : value) {
        list.push_back(v->unwrapped());
    }
    return list;
}

ResolveStatus SimpleConfigList::resolveStatus() {
    return ResolveStatusEnum::fromBool(resolved);
}

SimpleConfigListPtr SimpleConfigList::modify(const NoExceptionsModifierPtr& modifier, ResolveStatus newResolveStatus) {
    try {
        return modifyMayThrow(modifier, newResolveStatus);
    }
    catch (ConfigException&) {
        throw;
    }
    catch (std::exception& e) {
        throw ConfigExceptionBugOrBroken(std::string("unexpected checked exception:") + e.what());
    }
}

SimpleConfigListPtr SimpleConfigList::modifyMayThrow(const ModifierPtr& modifier, ResolveStatus newResolveStatus) {
    // lazy-create for optimization
    boost::optional<VectorAbstractConfigValue> changed;
    uint32_t i = 0;
    for (auto& v : value) {
        auto modified = modifier->modifyChildMayThrow("", std::dynamic_pointer_cast<AbstractConfigValue>(v));

        if (!changed && modified != v) {
            changed = VectorAbstractConfigValue();
            for (uint32_t j = 0; j < i; ++j) {
                changed->push_back(std::dynamic_pointer_cast<AbstractConfigValue>(value[j]));
            }
        }

        // once the new list is created, all elements
        // have to go in it. if modifyChild returned
        // null, we drop that element.
        if (changed && modified) {
            changed->push_back(modified);
        }

        i++;
    }

    if (changed) {
        return SimpleConfigList::make_instance(origin(), *changed, newResolveStatus);
    }
    else {
        return shared_from_this();
    }
}

AbstractConfigValuePtr SimpleConfigList::resolveSubstitutions(const ResolveContextPtr& context) {
    if (resolved) {
        return shared_from_this();
    }

    if (context->isRestrictedToChild()) {
        // if a list restricts to a child path, then it has no child paths,
        // so nothing to do.
        return shared_from_this();
    }
    else {
        try {
            return modifyMayThrow(SimpleConfigListModifier::make_instance(context), ResolveStatus::RESOLVED);
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
}

SimpleConfigListModifier::SimpleConfigListModifier(const ResolveContextPtr& context) :
    context(context) {
}

AbstractConfigValuePtr SimpleConfigListModifier::modifyChildMayThrow(const std::string& keyOrNull, const AbstractConfigValuePtr& v) {
    return context->resolve(v);
}

SimpleConfigListNoExceptionsModifier::SimpleConfigListNoExceptionsModifier(const PathPtr& prefix) :
    prefix(prefix) {
}

AbstractConfigValuePtr SimpleConfigListNoExceptionsModifier::modifyChild(const std::string& keyOrNull, const AbstractConfigValuePtr& v) {
    return v->relativized(prefix);
}

AbstractConfigValuePtr SimpleConfigList::relativized(const PathPtr& prefix) {
    return modify(SimpleConfigListNoExceptionsModifier::make_instance(prefix), resolveStatus());
}

bool SimpleConfigList::canEqual(const ConfigVariant& other) {
    return instanceof<SimpleConfigList>(other);
}

bool SimpleConfigList::equals(const ConfigVariant& other) {
    // note that "origin" is deliberately NOT part of equality
    if (instanceof<SimpleConfigList>(other)) {
        return canEqual(other) &&
                this->value.size() == dynamic_get<SimpleConfigList>(other)->value.size() &&
                std::equal(this->value.begin(), this->value.end(), dynamic_get<SimpleConfigList>(other)->value.begin(),
                    [&](const VectorConfigValue::value_type& first, const VectorConfigValue::value_type& second) {
                        return configEquals<AbstractConfigValuePtr>()(std::dynamic_pointer_cast<AbstractConfigValue>(first), std::dynamic_pointer_cast<AbstractConfigValue>(second));
                });
    }
    else {
        return false;
    }
}

uint32_t SimpleConfigList::hashCode() {
    // note that "origin" is deliberately NOT part of equality
    size_t hash = 0;
    for (auto& v : value) {
        boost::hash_combine(hash, std::dynamic_pointer_cast<AbstractConfigValue>(v)->hashCode());
    }
    return static_cast<uint32_t>(hash);
}

void SimpleConfigList::render(std::string& s, uint32_t indent_, const ConfigRenderOptionsPtr& options) {
    if (value.empty()) {
        s += "[]";
    }
    else {
        s += "[";
        if (options->getFormatted()) {
            s += "\n";
        }
        for (auto& v : value) {
            if (options->getOriginComments()) {
                indent(s, indent_ + 1, options);
                s += "# ";
                s += v->origin()->description();
                s += "\n";
            }
            if (options->getComments()) {
                for (auto& comment : v->origin()->comments()) {
                    indent(s, indent_ + 1, options);
                    s += "# ";
                    s += comment;
                    s += "\n";
                }
            }
            indent(s, indent_ + 1, options);

            std::dynamic_pointer_cast<AbstractConfigValue>(v)->render(s, indent_ + 1, options);
            s += ",";
            if (options->getFormatted()) {
                s += "\n";
            }
        }
        s.resize(s.length() - 1); // chop or newline
        if (options->getFormatted()) {
            s.resize(s.length() - 1); // also chop comma
            s += "\n";
            indent(s, indent_, options);
        }
        s += "]";
    }
}

VectorConfigValue::const_iterator SimpleConfigList::begin() const {
    return value.begin();
}

VectorConfigValue::const_iterator SimpleConfigList::end() const {
    return value.end();
}

VectorConfigValue::const_reference SimpleConfigList::at(VectorConfigValue::size_type n) const {
    return value.at(n);
}

VectorConfigValue::const_reference SimpleConfigList::front() const {
    return value.front();
}

VectorConfigValue::const_reference SimpleConfigList::back() const {
    return value.back();
}

VectorConfigValue::const_reference SimpleConfigList::operator[](VectorConfigValue::size_type n) const {
    return value.at(n);
}

bool SimpleConfigList::empty() const {
    return value.empty();
}

VectorConfigValue::size_type SimpleConfigList::size() const {
    return value.size();
}

void SimpleConfigList::clear() {
    throw weAreImmutable("clear");
}

void SimpleConfigList::pop_back() {
    throw weAreImmutable("pop_back");
}

void SimpleConfigList::resize(VectorConfigValue::size_type n, const VectorConfigValue::value_type& val) {
    throw weAreImmutable("resize");
}

VectorConfigValue::const_iterator SimpleConfigList::erase(VectorConfigValue::const_iterator pos) {
    throw weAreImmutable("erase");
}

VectorConfigValue::const_iterator SimpleConfigList::insert(VectorConfigValue::const_iterator pos, const VectorConfigValue::value_type& val) {
    throw weAreImmutable("insert");
}

ConfigExceptionUnsupportedOperation SimpleConfigList::weAreImmutable(const std::string& method) {
    return ConfigExceptionUnsupportedOperation("ConfigList is immutable, you can't call List.'" + method + "'");
}

AbstractConfigValuePtr SimpleConfigList::newCopy(const ConfigOriginPtr& newOrigin) {
    return SimpleConfigList::make_instance(newOrigin, MiscUtils::dynamic_vector<VectorAbstractConfigValue>(value));
}

SimpleConfigListPtr SimpleConfigList::concatenate(const SimpleConfigListPtr& other) {
    auto combinedOrigin = SimpleConfigOrigin::mergeOrigins(origin(), other->origin());
    VectorConfigValue combined(value);
    combined.insert(combined.end(), other->value.begin(), other->value.end());
    return SimpleConfigList::make_instance(combinedOrigin, MiscUtils::dynamic_vector<VectorAbstractConfigValue>(combined));
}

}
