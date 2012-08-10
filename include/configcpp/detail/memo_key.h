/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef MEMO_KEY_H_
#define MEMO_KEY_H_

#include "configcpp/detail/config_base.h"

namespace config {

///
/// The key used to memoize already-traversed nodes when resolving substitutions.
///
class MemoKey : public ConfigBase {
public:
    CONFIG_CLASS(MemoKey);

    MemoKey(const AbstractConfigValuePtr& value, const PathPtr& restrictToChildOrNull);

    virtual uint32_t hashCode() override;
    virtual bool equals(const ConfigVariant& other) override;

private:
    AbstractConfigValuePtr value;
    PathPtr restrictToChildOrNull;
};

}

#endif // MEMO_KEY_H_

