/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/config_delayed_merge_object.h"
#include "configcpp/detail/config_delayed_merge.h"
#include "configcpp/config_exception.h"
#include "configcpp/config_origin.h"
#include "configcpp/config_list.h"

namespace config {

ConfigDelayedMergeObject::ConfigDelayedMergeObject(const ConfigOriginPtr& origin, const VectorAbstractConfigValue& stack) :
    AbstractConfigObject(origin),
    stack(stack) {

    if (stack.empty()) {
        throw ConfigExceptionBugOrBroken("creating empty delayed merge object");
    }
    if (!instanceof<AbstractConfigObject>(stack.front())) {
        throw ConfigExceptionBugOrBroken("created a delayed merge object not guaranteed to be an object");
    }

    for (auto& v : stack) {
        if (instanceof<ConfigDelayedMerge>(v) || instanceof<ConfigDelayedMergeObject>(v)) {
            throw ConfigExceptionBugOrBroken("placed nested DelayedMerge in a ConfigDelayedMergeObject, should have consolidated stack");
        }
    }
}

AbstractConfigObjectPtr ConfigDelayedMergeObject::newCopy(ResolveStatus status, const ConfigOriginPtr& origin) {
    if (status != resolveStatus()) {
        throw ConfigExceptionBugOrBroken("attempt to create resolved ConfigDelayedMergeObject");
    }
    return make_instance(origin, stack);
}

AbstractConfigValuePtr ConfigDelayedMergeObject::resolveSubstitutions(const ResolveContextPtr& context) {
    auto merged = ConfigDelayedMerge::resolveSubstitutions(shared_from_this(), stack, context);
    if (instanceof<AbstractConfigObject>(merged)) {
        return std::static_pointer_cast<AbstractConfigObject>(merged);
    }
    else {
        throw ConfigExceptionBugOrBroken("somehow brokenly merged an object and didn't get an object, got " + merged->toString());
    }
}

ResolveReplacerPtr ConfigDelayedMergeObject::makeReplacer(uint32_t skipping) {
    return ConfigDelayedMergeObjectResolveReplacer::make_instance(stack, skipping);
}

ConfigDelayedMergeObjectResolveReplacer::ConfigDelayedMergeObjectResolveReplacer(const VectorAbstractConfigValue& stack, uint32_t skipping) :
    stack(stack),
    skipping(skipping) {
}

AbstractConfigValuePtr ConfigDelayedMergeObjectResolveReplacer::makeReplacement(const ResolveContextPtr& context) {
    return ConfigDelayedMerge::makeReplacement(context, stack, skipping);
}

ResolveStatus ConfigDelayedMergeObject::resolveStatus() {
    return ResolveStatus::UNRESOLVED;
}

AbstractConfigValuePtr ConfigDelayedMergeObject::relativized(const PathPtr& prefix) {
    VectorAbstractConfigValue newStack;
    for (auto& o : stack) {
        newStack.push_back(o->relativized(prefix));
    }
    return make_instance(origin(), newStack);
}

bool ConfigDelayedMergeObject::ignoresFallbacks() {
    return ConfigDelayedMerge::stackIgnoresFallbacks(stack);
}

AbstractConfigValuePtr ConfigDelayedMergeObject::mergedWithTheUnmergeable(const UnmergeablePtr& fallback) {
    return std::static_pointer_cast<ConfigDelayedMergeObject>(mergedWithTheUnmergeable(stack, fallback));
}

AbstractConfigValuePtr ConfigDelayedMergeObject::mergedWithObject(const AbstractConfigObjectPtr& fallback) {
    return mergedWithNonObject(fallback);
}

AbstractConfigValuePtr ConfigDelayedMergeObject::mergedWithNonObject(const AbstractConfigValuePtr& fallback) {
    return std::static_pointer_cast<ConfigDelayedMergeObject>(mergedWithNonObject(stack, fallback));
}

ConfigMergeablePtr ConfigDelayedMergeObject::withFallback(const ConfigMergeablePtr& mergeable) {
    return AbstractConfigObject::withFallback(mergeable);
}

ConfigObjectPtr ConfigDelayedMergeObject::withOnlyKey(const std::string& key) {
    throw notResolved();
}

ConfigObjectPtr ConfigDelayedMergeObject::withoutKey(const std::string& key) {
    throw notResolved();
}

AbstractConfigObjectPtr ConfigDelayedMergeObject::withOnlyPathOrNull(const PathPtr& path) {
    throw notResolved();
}

AbstractConfigObjectPtr ConfigDelayedMergeObject::withOnlyPath(const PathPtr& path) {
    throw notResolved();
}

AbstractConfigObjectPtr ConfigDelayedMergeObject::withoutPath(const PathPtr& path) {
    throw notResolved();
}

VectorAbstractConfigValue ConfigDelayedMergeObject::unmergedValues() {
    return stack;
}

bool ConfigDelayedMergeObject::canEqual(const ConfigVariant& other) {
    return instanceof<ConfigDelayedMergeObject>(other);
}

bool ConfigDelayedMergeObject::equals(const ConfigVariant& other) {
    // note that "origin" is deliberately NOT part of equality
    if (instanceof<ConfigDelayedMergeObject>(other)) {
        return canEqual(other) &&
                this->stack.size() == dynamic_get<ConfigDelayedMergeObject>(other)->stack.size() &&
                std::equal(this->stack.begin(), this->stack.end(), dynamic_get<ConfigDelayedMergeObject>(other)->stack.begin(), configEquals<AbstractConfigValuePtr>());
    }
    else {
        return false;
    }
}

uint32_t ConfigDelayedMergeObject::hashCode() {
    // note that "origin" is deliberately NOT part of equality
    size_t hash = 0;
    for (auto& v : stack) {
        boost::hash_combine(hash, v->hashCode());
    }
    return static_cast<uint32_t>(hash);
}

void ConfigDelayedMergeObject::render(std::string& s, uint32_t indent, const boost::optional<std::string>& atKey, const ConfigRenderOptionsPtr& options) {
    ConfigDelayedMerge::render(stack, s, indent, atKey, options);
}

void ConfigDelayedMergeObject::render(std::string& s, uint32_t indent, const ConfigRenderOptionsPtr& options) {
    render(s, indent, nullptr, options);
}

ConfigExceptionNotResolved ConfigDelayedMergeObject::notResolved() {
    return ConfigExceptionNotResolved("need to Config#resolve() before using this object, see the API docs for Config::resolve()");
}

ConfigVariant ConfigDelayedMergeObject::unwrapped() {
    throw notResolved();
}

ConfigValuePtr ConfigDelayedMergeObject::get(const std::string& key) {
    throw notResolved();
}

MapConfigValue::const_iterator ConfigDelayedMergeObject::begin() const {
    throw notResolved();
}

MapConfigValue::const_iterator ConfigDelayedMergeObject::end() const {
    throw notResolved();
}

MapConfigValue::mapped_type ConfigDelayedMergeObject::operator[](const MapConfigValue::key_type& key) const {
    throw notResolved();
}

bool ConfigDelayedMergeObject::empty() const {
    throw notResolved();
}

MapConfigValue::size_type ConfigDelayedMergeObject::size() const {
    throw notResolved();
}

MapConfigValue::size_type ConfigDelayedMergeObject::count(const MapConfigValue::key_type& key) const {
    throw notResolved();
}

MapConfigValue::const_iterator ConfigDelayedMergeObject::find(const MapConfigValue::key_type& key) const {
    throw notResolved();
}

AbstractConfigValuePtr ConfigDelayedMergeObject::attemptPeekWithPartialResolve(const std::string& key) {
    // a partial resolve of a ConfigDelayedMergeObject always results in a
    // SimpleConfigObject because all the substitutions in the stack get
    // resolved in order to look up the partial.
    // So we know here that we have not been resolved at all even
    // partially.
    // Given that, all this code is probably gratuitous, since the app code
    // is likely broken. But in general we only throw NotResolved if you try
    // to touch the exact key that isn't resolved, so this is in that
    // spirit.

    // we'll be able to return a key if we have a value that ignores
    // fallbacks, prior to any unmergeable values.
    for (auto& layer : stack) {
        if (instanceof<AbstractConfigObject>(layer)) {
            auto objectLayer = std::static_pointer_cast<AbstractConfigObject>(layer);
            auto v = objectLayer->attemptPeekWithPartialResolve(key);
            if (v) {
                if (v->ignoresFallbacks()) {
                    // we know we won't need to merge anything in to this value
                    return v;
                }
                else {
                    // we can't return this value because we know there are
                    // unmergeable values later in the stack that may
                    // contain values that need to be merged with this
                    // value. we'll throw the exception when we get to those
                    // unmergeable values, so continue here.
                    continue;
                }
            }
            else if (instanceof<Unmergeable>(layer)) {
                // an unmergeable object (which would be another
                // ConfigDelayedMergeObject) can't know that a key is
                // missing, so it can't return null; it can only return a
                // value or throw NotPossibleToResolve
                throw ConfigExceptionBugOrBroken("should not be reached: unmergeable object returned null value");
            }
            else {
                // a non-unmergeable AbstractConfigObject that returned null
                // for the key in question is not relevant, we can keep
                // looking for a value.
                continue;
            }
        }
        else if (instanceof<Unmergeable>(layer)) {
            throw ConfigExceptionNotResolved("Key '" + key + "' is not available at '" +
                    origin()->description() + "' because value at '" +
                    layer->origin()->description() +
                    "' has not been resolved and may turn out to contain or hide '" +
                    key + "'." +
                    " Be sure to Config#resolve() before using a config object.");
        }
        else if (layer->resolveStatus() == ResolveStatus::UNRESOLVED) {
            // if the layer is not an object, and not a substitution or merge,
            // then it's something that's unresolved because it _contains_
            // an unresolved object... i.e. it's an array
            if (!instanceof<ConfigList>(layer)) {
                throw ConfigExceptionBugOrBroken("Expecting a list here, not " + layer->toString());
            }
            // all later objects will be hidden so we can say we won't find the key
            return nullptr;
        }
        else {
            // non-object, but resolved, like an integer or something.
            // has no children so the one we're after won't be in it.
            // we would only have this in the stack in case something
            // else "looks back" to it due to a cycle.
            // anyway at this point we know we can't find the key anymore.
            if (!layer->ignoresFallbacks()) {
                throw ConfigExceptionBugOrBroken("resolved non-object should ignore fallbacks");
            }
            return nullptr;
        }
    }
    // If we get here, then we never found anything unresolved which means
    // the ConfigDelayedMergeObject should not have existed. some
    // invariant was violated.
    throw ConfigExceptionBugOrBroken("Delayed merge stack does not contain any unmergeable values");
}

}
