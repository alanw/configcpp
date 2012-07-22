/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef FULL_INCLUDER_H_
#define FULL_INCLUDER_H_

#include "configcpp/config_includer.h"
#include "configcpp/config_includer_file.h"

namespace config {

class FullIncluder : public virtual ConfigIncluder, public virtual ConfigIncluderFile {
};

}

#endif // FULL_INCLUDER_H_

