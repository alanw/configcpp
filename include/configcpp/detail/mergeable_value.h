/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef MERGEABLE_VALUE_H_
#define MERGEABLE_VALUE_H_

#include "configcpp/config_mergeable.h"

namespace config {

class MergeableValue : public virtual ConfigMergeable {
public:
    /// Converts a Config to its root object and a ConfigValue to itself
    virtual ConfigValuePtr toFallbackValue() = 0;
};

}

#endif // MERGEABLE_VALUE_H_

