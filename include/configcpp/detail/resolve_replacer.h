/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef RESOLVE_REPLACER_H_
#define RESOLVE_REPLACER_H_

#include "configcpp/detail/config_base.h"

namespace config {

///
/// Callback that generates a replacement to use for resolving a substitution.
///
class ResolveReplacer {
public:
    virtual AbstractConfigValuePtr replace(const ResolveContextPtr& context);

protected:
    virtual AbstractConfigValuePtr makeReplacement(const ResolveContextPtr& context) = 0;

public:
    static ResolveReplacerPtr cycleResolveReplacer();

private:
    // this is a "lazy val" in essence (we only want to
    // make the replacement one time). Making it volatile
    // is good enough for thread safety as long as this
    // cache is only an optimization and making the replacement
    // twice has no side effects, which it should not...
    AbstractConfigValuePtr replacement;
};

class ResolveReplacerCycle : public virtual ResolveReplacer, public ConfigBase {
public:
    CONFIG_CLASS(ResolveReplacerCycle);

protected:
    virtual AbstractConfigValuePtr makeReplacement(const ResolveContextPtr& context) override;
};

}

#endif // RESOLVE_REPLACER_H_

