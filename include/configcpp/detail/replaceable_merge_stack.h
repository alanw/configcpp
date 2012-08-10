/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef REPLACEABLE_MERGE_STACK_H_
#define REPLACEABLE_MERGE_STACK_H_

#include "configcpp/config_types.h"

namespace config {

///
/// Implemented by a merge stack (ConfigDelayedMerge, ConfigDelayedMergeObject)
/// that replaces itself during substitution resolution in order to implement
/// "look backwards only" semantics.
///
class ReplaceableMergeStack {
public:
    /// Make a replacer for this object, skipping the given number of items in
    /// the stack.
    virtual ResolveReplacerPtr makeReplacer(uint32_t skipping) = 0;
};

}

#endif // REPLACEABLE_MERGE_STACK_H_

