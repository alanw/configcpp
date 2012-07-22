/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef DEFAULT_TRANSFORMER_H_
#define DEFAULT_TRANSFORMER_H_

#include "configcpp/config_types.h"

namespace config {

///
/// Default automatic type transformations.
///
class DefaultTransformer {
public:
    static AbstractConfigValuePtr transform(const AbstractConfigValuePtr& value, ConfigValueType requested);
};

}

#endif // DEFAULT_TRANSFORMER_H_
