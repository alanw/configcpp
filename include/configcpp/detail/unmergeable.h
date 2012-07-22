/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef UNMERGEABLE_H_
#define UNMERGEABLE_H_

#include "configcpp/config_types.h"

namespace config {

///
/// Interface that tags a ConfigValue that is not mergeable until after
/// substitutions are resolved. Basically these are special ConfigValue that
/// never appear in a resolved tree, like {@link ConfigSubstitution} and
/// {@link ConfigDelayedMerge}.
///
class Unmergeable {
public:
    virtual VectorAbstractConfigValue unmergedValues() = 0;
};

}

#endif // UNMERGEABLE_H_
