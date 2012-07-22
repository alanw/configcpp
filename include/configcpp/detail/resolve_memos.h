/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef RESOLVE_MEMOS_H_
#define RESOLVE_MEMOS_H_

#include "configcpp/detail/config_base.h"

namespace config {

///
/// This exists because we have to memoize resolved substitutions as we go
/// through the config tree; otherwise we could end up creating multiple copies
/// of values or whole trees of values as we follow chains of substitutions.
///
class ResolveMemos : public ConfigBase {
public:
    CONFIG_CLASS(ResolveMemos);

    AbstractConfigValuePtr get(const MemoKeyPtr& key);
    void put(const MemoKeyPtr& key, const AbstractConfigValuePtr& value);

private:
    // note that we can resolve things to undefined (represented as null,
    // rather than ConfigNull) so this map can have null values.
    MapMemoKeyAbstractConfigValue memos;
};

}

#endif // RESOLVE_MEMOS_H_
