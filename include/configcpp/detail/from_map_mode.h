/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef FROM_MAP_MODE_H_
#define FROM_MAP_MODE_H_

#include "configcpp/config_types.h"

namespace config {

enum class FromMapMode : uint32_t {
    KEYS_ARE_PATHS, KEYS_ARE_KEYS
};

}

#endif // FROM_MAP_MODE_H_

