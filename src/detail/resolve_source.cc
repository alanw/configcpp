/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/resolve_source.h"
#include "configcpp/detail/resolve_context.h"
#include "configcpp/detail/resolve_replacer.h"
#include "configcpp/detail/abstract_config_object.h"
#include "configcpp/detail/substitution_expression.h"
#include "configcpp/detail/path.h"
#include "configcpp/detail/config_impl.h"
#include "configcpp/config_resolve_options.h"

namespace config {

ResolveSource::ResolveSource(const AbstractConfigObjectPtr& root) :
    root(root) {
}

AbstractConfigValuePtr ResolveSource::findInObject(const AbstractConfigObjectPtr& obj, const ResolveContextPtr& context, const SubstitutionExpressionPtr& subst) {
    return obj->peekPath(subst->path(), context);
}

AbstractConfigValuePtr ResolveSource::lookupSubst(const ResolveContextPtr& context, const SubstitutionExpressionPtr& subst, uint32_t prefixLength) {
    context->trace(subst);
    ConfigExceptionPtr finally;
    AbstractConfigValuePtr result;
    try {
        // First we look up the full path, which means relative to the
        // included file if we were not a root file
        result = findInObject(root, context, subst);

        if (!result) {
            // Then we want to check relative to the root file. We don't
            // want the prefix we were included at to be used when looking
            // up env variables either.
            auto unprefixed = subst->changePath(subst->path()->subPath(prefixLength));

            // replace the debug trace path
            context->untrace();
            context->trace(unprefixed);

            if (prefixLength > 0) {
                result = findInObject(root, context, unprefixed);
            }

            if (!result && context->options()->getUseSystemEnvironment()) {
                result = findInObject(ConfigImpl::envVariablesAsConfigObject(), context, unprefixed);
            }
        }

        if (result) {
            result = context->resolve(result);
        }
    }
    catch (ConfigException& e) {
        finally = e.clone();
    }

    context->untrace();

    if (finally) {
        finally->raise();
    }

    return result;
}

void ResolveSource::replace(const AbstractConfigValuePtr& value, const ResolveReplacerPtr& replacer) {
    auto old = replacements.insert(std::make_pair(value, replacer));
    if (!old.second) {
        throw ConfigExceptionBugOrBroken("should not have replaced the same value twice: " + value->toString());
    }
}

void ResolveSource::unreplace(const AbstractConfigValuePtr& value) {
    if (replacements.erase(value) == 0) {
        throw ConfigExceptionBugOrBroken("unreplace() without replace(): " + value->toString());
    }
}

AbstractConfigValuePtr ResolveSource::replacement(const ResolveContextPtr& context, const AbstractConfigValuePtr& value) {
    auto replacer = replacements.find(value);
    if (replacer == replacements.end()) {
        return value;
    }
    else {
        return replacer->second->replace(context);
    }
}

AbstractConfigValuePtr ResolveSource::resolveCheckingReplacement(const ResolveContextPtr& context, const AbstractConfigValuePtr& original) {
    auto replacement_ = replacement(context, original);

    if (replacement_ != original) {
        // start over, checking if replacement was memoized
        return context->resolve(replacement_);
    }
    else {
        auto resolved = original->resolveSubstitutions(context);
        return resolved;
    }
}

}
