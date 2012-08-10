/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef RESOLVE_SOURCE_H_
#define RESOLVE_SOURCE_H_

#include "configcpp/detail/config_base.h"

namespace config {

///
/// This class is the source for values for a substitution like ${foo}.
///
class ResolveSource : public ConfigBase {
public:
    CONFIG_CLASS(ResolveSource);

    ResolveSource(const AbstractConfigObjectPtr& root);

private:
    static AbstractConfigValuePtr findInObject(const AbstractConfigObjectPtr& obj,
                                               const ResolveContextPtr& context,
                                               const SubstitutionExpressionPtr& subst);

public:
    AbstractConfigValuePtr lookupSubst(const ResolveContextPtr& context,
                                       const SubstitutionExpressionPtr& subst,
                                       uint32_t prefixLength);

    void replace(const AbstractConfigValuePtr& value, const ResolveReplacerPtr& replacer);
    void unreplace(const AbstractConfigValuePtr& value);

private:
    AbstractConfigValuePtr replacement(const ResolveContextPtr& context,
                                       const AbstractConfigValuePtr& value);

public:
    /// Conceptually, this is key.value().resolveSubstitutions() but using the
    /// replacement for key.value() if any.
    AbstractConfigValuePtr resolveCheckingReplacement(const ResolveContextPtr& context,
                                                      const AbstractConfigValuePtr& original);

private:
    AbstractConfigObjectPtr root;

    // Conceptually, we transform the ResolveSource whenever we traverse
    // a substitution or delayed merge stack, in order to remove the
    // traversed node and therefore avoid circular dependencies.
    // We implement it with this somewhat hacky "patch a replacement"
    // mechanism instead of actually transforming the tree.
    MapResolveReplacer replacements;
};

}

#endif // RESOLVE_SOURCE_H_

