/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/resolve_memos.h"
#include "configcpp/detail/memo_key.h"

namespace config {

AbstractConfigValuePtr ResolveMemos::get(const MemoKeyPtr& key) {
    auto value = memos.find(key);
    return value == memos.end() ? nullptr : value->second;
}

void ResolveMemos::put(const MemoKeyPtr& key, const AbstractConfigValuePtr& value) {
    memos[key] = value;
}

}
