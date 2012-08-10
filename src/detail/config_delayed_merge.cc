/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/config_delayed_merge.h"
#include "configcpp/detail/config_delayed_merge_object.h"
#include "configcpp/detail/resolve_context.h"
#include "configcpp/detail/resolve_source.h"
#include "configcpp/detail/config_impl_util.h"
#include "configcpp/config_render_options.h"
#include "configcpp/config_origin.h"
#include "configcpp/config_exception.h"
#include "configcpp/config_value_type.h"

namespace config {

ConfigDelayedMerge::ConfigDelayedMerge(const ConfigOriginPtr& origin, const VectorAbstractConfigValue& stack) :
    AbstractConfigValue(origin),
    stack(stack) {
    if (stack.empty()) {
        throw ConfigExceptionBugOrBroken("creating empty delayed merge value");
    }
    for (auto& v : stack) {
        if (instanceof<ConfigDelayedMerge>(v) || instanceof<ConfigDelayedMergeObject>(v)) {
            throw ConfigExceptionBugOrBroken("placed nested DelayedMerge in a ConfigDelayedMerge, should have consolidated stack");
        }
    }
}

ConfigValueType ConfigDelayedMerge::valueType() {
    throw ConfigExceptionNotResolved("called valueType() on value with unresolved substitutions, need to Config::resolve() first, see API docs");
}

ConfigVariant ConfigDelayedMerge::unwrapped() {
    throw ConfigExceptionNotResolved("called unwrapped() on value with unresolved substitutions, need to Config::resolve() first, see API docs");
}

AbstractConfigValuePtr ConfigDelayedMerge::resolveSubstitutions(const ResolveContextPtr& context) {
    return resolveSubstitutions(shared_from_this(), stack, context);
}

AbstractConfigValuePtr ConfigDelayedMerge::resolveSubstitutions(const ReplaceableMergeStackPtr& replaceable, const VectorAbstractConfigValue& stack, const ResolveContextPtr& context) {
    // to resolve substitutions, we need to recursively resolve
    // the stack of stuff to merge, and merge the stack so
    // we won't be a delayed merge anymore. If restrictToChildOrNull
    // is non-null, we may remain a delayed merge though.

    uint32_t count = 0;
    AbstractConfigValuePtr merged;
    for (auto& v : stack) {
        if (instanceof<ReplaceableMergeStack>(v)) {
            throw ConfigExceptionBugOrBroken("A delayed merge should not contain another one: " + boost::lexical_cast<std::string>(replaceable.get()));
        }

        bool replaced = false;

        // we only replace if we have a substitution, or
        // value-concatenation containing one. The Unmergeable
        // here isn't a delayed merge stack since we can't contain
        // another stack (see assertion above).
        if (instanceof<Unmergeable>(v)) {
            // If, while resolving 'v' we come back to the same
            // merge stack, we only want to look _below_ 'v'
            // in the stack. So we arrange to replace the
            // ConfigDelayedMerge with a value that is only
            // the remainder of the stack below this one.
            context->source()->replace(std::dynamic_pointer_cast<AbstractConfigValue>(replaceable), replaceable->makeReplacer(count + 1));
            replaced = true;
        }

        AbstractConfigValuePtr resolved;
        ConfigExceptionPtr finally;
        try {
            resolved = context->resolve(v);
        }
        catch (ConfigException& e) {
            finally = e.clone();
        }

        if (replaced) {
            context->source()->unreplace(std::dynamic_pointer_cast<AbstractConfigValue>(replaceable));
        }

        if (finally) {
            finally->raise();
        }

        if (resolved) {
            if (!merged) {
                merged = resolved;
            }
            else {
                merged = std::dynamic_pointer_cast<AbstractConfigValue>(merged->withFallback(resolved));
            }
        }
        count += 1;
    }

    return merged;
}

ResolveReplacerPtr ConfigDelayedMerge::makeReplacer(uint32_t skipping) {
    return ConfigDelayedMergeResolveReplacer::make_instance(stack, skipping);
}

ConfigDelayedMergeResolveReplacer::ConfigDelayedMergeResolveReplacer(const VectorAbstractConfigValue& stack, uint32_t skipping) :
    stack(stack),
    skipping(skipping) {
}

AbstractConfigValuePtr ConfigDelayedMergeResolveReplacer::makeReplacement(const ResolveContextPtr& context) {
    return ConfigDelayedMerge::makeReplacement(context, stack, skipping);
}

AbstractConfigValuePtr ConfigDelayedMerge::makeReplacement(const ResolveContextPtr& context, const VectorAbstractConfigValue& stack, uint32_t skipping) {
    VectorAbstractConfigValue subStack(stack.begin() + skipping, stack.end());

    if (subStack.empty()) {
        throw NotPossibleToResolve(context);
    }
    else {
        // generate a new merge stack from only the remaining items
        AbstractConfigValuePtr merged;
        for (auto& v : subStack) {
            if (!merged) {
                merged = v;
            }
            else {
                merged = std::dynamic_pointer_cast<AbstractConfigValue>(merged->withFallback(v));
            }
        }
        return merged;
    }
}

ResolveStatus ConfigDelayedMerge::resolveStatus() {
    return ResolveStatus::UNRESOLVED;
}

AbstractConfigValuePtr ConfigDelayedMerge::relativized(const PathPtr& prefix) {
    VectorAbstractConfigValue newStack;
    for (auto& o : stack) {
        newStack.push_back(o->relativized(prefix));
    }
    return make_instance(origin(), newStack);
}

bool ConfigDelayedMerge::stackIgnoresFallbacks(const VectorAbstractConfigValue& stack) {
    auto last = stack.back();
    return last->ignoresFallbacks();
}

bool ConfigDelayedMerge::ignoresFallbacks() {
    return stackIgnoresFallbacks(stack);
}

AbstractConfigValuePtr ConfigDelayedMerge::newCopy(const ConfigOriginPtr& newOrigin) {
    return make_instance(newOrigin, stack);
}

AbstractConfigValuePtr ConfigDelayedMerge::mergedWithTheUnmergeable(const UnmergeablePtr& fallback) {
    return mergedWithTheUnmergeable(stack, fallback);
}

AbstractConfigValuePtr ConfigDelayedMerge::mergedWithObject(const AbstractConfigObjectPtr& fallback) {
    return mergedWithObject(stack, fallback);
}

AbstractConfigValuePtr ConfigDelayedMerge::mergedWithNonObject(const AbstractConfigValuePtr& fallback) {
    return mergedWithNonObject(stack, fallback);
}

VectorAbstractConfigValue ConfigDelayedMerge::unmergedValues() {
    return stack;
}

bool ConfigDelayedMerge::canEqual(const ConfigVariant& other) {
    return instanceof<ConfigDelayedMerge>(other);
}

bool ConfigDelayedMerge::equals(const ConfigVariant& other) {
    // note that "origin" is deliberately NOT part of equality
    if (instanceof<ConfigDelayedMerge>(other)) {
        return canEqual(other) &&
                this->stack.size() == dynamic_get<ConfigDelayedMerge>(other)->stack.size() &&
                std::equal(this->stack.begin(), this->stack.end(), dynamic_get<ConfigDelayedMerge>(other)->stack.begin(), configEquals<AbstractConfigValuePtr>());
    }
    else {
        return false;
    }
}

uint32_t ConfigDelayedMerge::hashCode() {
    // note that "origin" is deliberately NOT part of equality
    size_t hash = 0;
    for (auto& v : stack) {
        boost::hash_combine(hash, v->hashCode());
    }
    return static_cast<uint32_t>(hash);
}

void ConfigDelayedMerge::render(std::string& s, uint32_t indent, const boost::optional<std::string>& atKey, const ConfigRenderOptionsPtr& options) {
    render(stack, s, indent, atKey, options);
}

void ConfigDelayedMerge::render(const VectorAbstractConfigValue& stack, std::string& s, uint32_t indent_, const boost::optional<std::string>& atKey, const ConfigRenderOptionsPtr& options) {
    bool commentMerge = options->getComments();
    if (commentMerge) {
        s += "# unresolved merge of " + boost::lexical_cast<std::string>(stack.size()) + " values follows (\n";
        if (!atKey) {
            indent(s, indent_, options);
            s += "# this unresolved merge will not be parseable because it's at the root of the object\n";
            indent(s, indent_, options);
            s += "# the HOCON format has no way to list multiple root objects in a single file\n";
        }
    }

    VectorAbstractConfigValue reversed(stack.rbegin(), stack.rend());

    uint32_t i = 0;
    for (auto& v : reversed) {
        if (commentMerge) {
            indent(s, indent_, options);
            if (atKey) {
                s += "#     unmerged value " + boost::lexical_cast<std::string>(i) + " for key " + ConfigImplUtil::renderJsonString(*atKey) + " from ";
            }
            else {
                s += "#     unmerged value " + boost::lexical_cast<std::string>(i) + " from ";
            }
            i += 1;
            s += v->origin()->description() + "\n";

            for (auto& comment : v->origin()->comments()) {
                indent(s, indent_, options);
                s += "# " + comment + "\n";
            }
        }
        indent(s, indent_, options);

        if (atKey) {
            s += ConfigImplUtil::renderJsonString(*atKey);
            if (options->getFormatted()) {
                s += " : ";
            }
            else {
                s += ":";
            }
        }
        v->render(s, indent_, options);
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
    }
    if (commentMerge) {
        indent(s, indent_, options);
        s += "# ) end of unresolved merge\n";
    }
}

}
