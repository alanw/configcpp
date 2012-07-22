/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef RESOLVE_CONTEXT_H_
#define RESOLVE_CONTEXT_H_

#include "configcpp/detail/config_base.h"

namespace config {

class ResolveContext : public ConfigBase {
public:
    CONFIG_CLASS(ResolveContext);

    ResolveContext(const ResolveSourcePtr& source,
                   const ResolveMemosPtr& memos,
                   const ConfigResolveOptionsPtr& options,
                   const PathPtr& restrictToChild,
                   const VectorSubstitutionExpression& expressionTrace);
    ResolveContext(const AbstractConfigObjectPtr& root,
                   const ConfigResolveOptionsPtr& options,
                   const PathPtr& restrictToChild);

    ResolveSourcePtr source();
    ConfigResolveOptionsPtr options();

    bool isRestrictedToChild();
    PathPtr restrictToChild();
    ResolveContextPtr restrict(const PathPtr& restrictTo);
    ResolveContextPtr unrestricted();

    void trace(const SubstitutionExpressionPtr& expr);
    void untrace();
    std::string traceString();

    AbstractConfigValuePtr resolve(const AbstractConfigValuePtr& original);
    static AbstractConfigValuePtr resolve(const AbstractConfigValuePtr& value,
                                          const AbstractConfigObjectPtr& root,
                                          const ConfigResolveOptionsPtr& options,
                                          const PathPtr& restrictToChildOrNull = nullptr);

private:
    /// This is unfortunately mutable so should only be shared among
    /// ResolveContext in the same traversal.
    ResolveSourcePtr source_;

    /// This is unfortunately mutable so should only be shared among
    /// ResolveContext in the same traversal.
    ResolveMemosPtr memos;

    ConfigResolveOptionsPtr options_;

    /// The current path restriction, used to ensure lazy
    /// resolution and avoid gratuitous cycles. without this,
    /// any sibling of an object we're traversing could
    /// cause a cycle "by side effect"
    /// CAN BE NULL for a full resolve.
    PathPtr restrictToChild_;

    /// Another mutable unfortunate. This is
    /// used to make nice error messages when
    /// resolution fails.
    VectorSubstitutionExpression expressionTrace;
};

}

#endif // RESOLVE_CONTEXT_H_

