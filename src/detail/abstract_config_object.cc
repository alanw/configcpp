/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/abstract_config_object.h"
#include "configcpp/detail/simple_config.h"
#include "configcpp/detail/config_impl.h"
#include "configcpp/detail/resolve_context.h"
#include "configcpp/detail/path.h"
#include "configcpp/detail/config_delayed_merge_object.h"
#include "configcpp/detail/simple_config_origin.h"
#include "configcpp/config_exception.h"
#include "configcpp/config_value_type.h"

namespace config {

AbstractConfigObject::AbstractConfigObject(const ConfigOriginPtr& origin) :
    AbstractConfigValue(origin) {
}

void AbstractConfigObject::initialize() {
    config = SimpleConfig::make_instance(shared_from_this());
}

ConfigPtr AbstractConfigObject::toConfig() {
    return config;
}

ConfigValuePtr AbstractConfigObject::toFallbackValue() {
    return shared_from_this();
}

AbstractConfigValuePtr AbstractConfigObject::peekAssumingResolved(const std::string& key, const PathPtr& originalPath) {
    try {
        return attemptPeekWithPartialResolve(key);
    }
    catch (ConfigExceptionNotResolved& e) {
        throw ConfigImpl::improveNotResolved(originalPath, e);
    }
}

AbstractConfigValuePtr AbstractConfigObject::peekPath(const PathPtr& path, const ResolveContextPtr& context) {
    return peekPath(shared_from_this(), path, context);
}

AbstractConfigValuePtr AbstractConfigObject::peekPath(const PathPtr& path) {
    try {
        return peekPath(shared_from_this(), path, nullptr);
    }
    catch (NotPossibleToResolve&) {
        throw ConfigExceptionBugOrBroken("NotPossibleToResolve happened though we had no ResolveContext in peekPath");
    }
}

AbstractConfigValuePtr AbstractConfigObject::peekPath(const AbstractConfigObjectPtr& self, const PathPtr& path, const ResolveContextPtr& context) {
    try {
        if (context) {
            // walk down through the path resolving only things along that
            // path, and then recursively call ourselves with no resolve
            // context.
            auto partiallyResolved = context->restrict(path)->resolve(self);
            if (instanceof<AbstractConfigObject>(partiallyResolved)) {
                return peekPath(std::static_pointer_cast<AbstractConfigObject>(partiallyResolved), path, nullptr);
            }
            else {
                throw ConfigExceptionBugOrBroken("resolved object to non-object " + self->toString() + " to " + partiallyResolved->toString());
            }
        }
        else {
            // with no resolver, we'll fail if anything along the path can't
            // be looked at without resolving.
            auto next = path->remainder();
            auto v = self->attemptPeekWithPartialResolve(path->first());

            if (!next) {
                return v;
            }
            else {
                if (instanceof<AbstractConfigObject>(v)) {
                    return peekPath(std::static_pointer_cast<AbstractConfigObject>(v), next, nullptr);
                }
                else {
                    return nullptr;
                }
            }
        }
    }
    catch (ConfigExceptionNotResolved& e) {
        throw ConfigImpl::improveNotResolved(path, e);
    }
}

ConfigValueType AbstractConfigObject::valueType() {
    return ConfigValueType::OBJECT;
}

AbstractConfigValuePtr AbstractConfigObject::newCopy(const ConfigOriginPtr& origin) {
    return newCopy(resolveStatus(), origin);
}

AbstractConfigValuePtr AbstractConfigObject::constructDelayedMerge(const ConfigOriginPtr& origin, const VectorAbstractConfigValue& stack) {
    return ConfigDelayedMergeObject::make_instance(origin, stack);
}

ConfigMergeablePtr AbstractConfigObject::withFallback(const ConfigMergeablePtr& mergeable) {
    return AbstractConfigValue::withFallback(mergeable);
}

ConfigOriginPtr AbstractConfigObject::mergeOrigins(const VectorAbstractConfigValue& stack) {
    if (stack.empty()) {
        throw ConfigExceptionBugOrBroken("can't merge origins on empty list");
    }
    VectorConfigOrigin origins;
    ConfigOriginPtr firstOrigin;
    uint32_t numMerged = 0;
    for (auto& v : stack) {
        if (!firstOrigin) {
            firstOrigin = v->origin();
        }
        if (instanceof<AbstractConfigObject>(v) &&
                std::static_pointer_cast<AbstractConfigObject>(v)->resolveStatus() == ResolveStatus::RESOLVED &&
                std::dynamic_pointer_cast<ConfigObject>(v)->empty()) {
            // don't include empty files or the .empty()
            // config in the description, since they are
            // likely to be "implementation details"
        }
        else {
            origins.push_back(v->origin());
            numMerged += 1;
        }
    }

    if (numMerged == 0) {
        // the configs were all empty, so just use the first one
        origins.push_back(firstOrigin);
    }

    return SimpleConfigOrigin::mergeOrigins(origins);
}

ConfigOriginPtr AbstractConfigObject::mergeOrigins(const VectorAbstractConfigObject& stack) {
    return mergeOrigins(VectorAbstractConfigValue(stack.begin(), stack.end()));
}

MapConfigValue::const_iterator AbstractConfigObject::begin() const {
    return ConfigObject::begin();
}

MapConfigValue::const_iterator AbstractConfigObject::end() const {
    return ConfigObject::end();
}

MapConfigValue::mapped_type AbstractConfigObject::operator[](const MapConfigValue::key_type& key) const {
    auto val = ConfigObject::find(key);
    return val == ConfigObject::end() ? nullptr : val->second;
}

bool AbstractConfigObject::empty() const {
    return ConfigObject::empty();
}

MapConfigValue::size_type AbstractConfigObject::size() const {
    return ConfigObject::size();
}

MapConfigValue::size_type AbstractConfigObject::count(const MapConfigValue::key_type& key) const {
    return ConfigObject::count(key);
}

MapConfigValue::const_iterator AbstractConfigObject::find(const MapConfigValue::key_type& key) const {
    return ConfigObject::find(key);
}

ConfigExceptionUnsupportedOperation AbstractConfigObject::weAreImmutable(const std::string& method) {
    return ConfigExceptionUnsupportedOperation("ConfigObject is immutable, you can't call Map." + method);
}

void AbstractConfigObject::clear() {
    throw weAreImmutable("clear");
}

MapConfigValue::size_type AbstractConfigObject::erase(const MapConfigValue::key_type&) {
    throw weAreImmutable("erase");
}

void AbstractConfigObject::erase(const MapConfigValue::const_iterator&) {
    throw weAreImmutable("erase");
}

void AbstractConfigObject::insert(const MapConfigValue::value_type&) {
    throw weAreImmutable("insert");
}

}
