/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/resolve_context.h"
#include "configcpp/detail/resolve_source.h"
#include "configcpp/detail/resolve_status.h"
#include "configcpp/detail/resolve_memos.h"
#include "configcpp/detail/abstract_config_value.h"
#include "configcpp/detail/path.h"
#include "configcpp/detail/memo_key.h"
#include "configcpp/detail/substitution_expression.h"
#include "configcpp/config_exception.h"

namespace config {

ResolveContext::ResolveContext(const ResolveSourcePtr& source,
                               const ResolveMemosPtr& memos,
                               const ConfigResolveOptionsPtr& options,
                               const PathPtr& restrictToChild,
                               const VectorSubstitutionExpression& expressionTrace) :
    source_(source),
    memos(memos),
    options_(options),
    restrictToChild_(restrictToChild),
    expressionTrace(expressionTrace) {
}

ResolveContext::ResolveContext(const AbstractConfigObjectPtr& root,
                               const ConfigResolveOptionsPtr& options,
                               const PathPtr& restrictToChild) :
    ResolveContext(ResolveSource::make_instance(root), ResolveMemos::make_instance(), options, restrictToChild, {}) {
}

ResolveSourcePtr ResolveContext::source() {
    return source_;
}

ConfigResolveOptionsPtr ResolveContext::options() {
    return options_;
}

bool ResolveContext::isRestrictedToChild() {
    return !!restrictToChild_;
}

PathPtr ResolveContext::restrictToChild() {
    return restrictToChild_;
}

ResolveContextPtr ResolveContext::restrict(const PathPtr& restrictTo) {
    if (restrictTo == restrictToChild_) {
        return shared_from_this();
    }
    else {
        return ResolveContext::make_instance(source_, memos, options_, restrictTo, expressionTrace);
    }
}

ResolveContextPtr ResolveContext::unrestricted() {
    return restrict(nullptr);
}

void ResolveContext::trace(const SubstitutionExpressionPtr& expr) {
    expressionTrace.push_back(expr);
}

void ResolveContext::untrace() {
    expressionTrace.pop_back();
}

std::string ResolveContext::traceString() {
    std::string separator = ", ";
    std::ostringstream stream;
    for (auto& expr : expressionTrace) {
        stream << expr->toString() << separator;
    }
    std::string trace = stream.str();
    if (!trace.empty()) {
        trace.resize(trace.length() - separator.length());
    }
    return trace;
}

AbstractConfigValuePtr ResolveContext::resolve(const AbstractConfigValuePtr& original) {
    // a fully-resolved (no restrictToChild) object can satisfy a
    // request for a restricted object, so always check that first.
    auto fullKey = MemoKey::make_instance(original, nullptr);
    MemoKeyPtr restrictedKey;

    auto cached = memos->get(fullKey);

    // but if there was no fully-resolved object cached, we'll only
    // compute the restrictToChild object so use a more limited
    // memo key
    if (!cached && isRestrictedToChild()) {
        restrictedKey = MemoKey::make_instance(original, restrictToChild());
        cached = memos->get(restrictedKey);
    }

    if (cached) {
        return cached;
    }
    else {
        auto resolved = source_->resolveCheckingReplacement(shared_from_this(), original);

        if (!resolved || resolved->resolveStatus() == ResolveStatus::RESOLVED) {
            // if the resolved object is fully resolved by resolving
            // only the restrictToChildOrNull, then it can be cached
            // under fullKey since the child we were restricted to
            // turned out to be the only unresolved thing.
            memos->put(fullKey, resolved);
        }
        else {
            // if we have an unresolved object then either we did a
            // partial resolve restricted to a certain child, or it's
            // a bug.
            if (isRestrictedToChild()) {
                if (!restrictedKey) {
                    throw ConfigExceptionBugOrBroken("restrictedKey should not be null here");
                }
                memos->put(restrictedKey, resolved);
            }
            else {
                throw ConfigExceptionBugOrBroken("resolveSubstitutions() did not give us a resolved object");
            }
        }

        return resolved;
    }
}

AbstractConfigValuePtr ResolveContext::resolve(const AbstractConfigValuePtr& value, const AbstractConfigObjectPtr& root, const ConfigResolveOptionsPtr& options, const PathPtr& restrictToChildOrNull) {
    auto context = ResolveContext::make_instance(root, options, nullptr);

    try {
        return context->resolve(value);
    }
    catch (NotPossibleToResolve&) {
        // ConfigReference was supposed to catch NotPossibleToResolve
        throw ConfigExceptionBugOrBroken("NotPossibleToResolve was thrown from an outermost resolve");
    }
}

}
