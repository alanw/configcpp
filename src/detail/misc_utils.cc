/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/misc_utils.h"

namespace config {

MiscUtils::MiscUtils() {
    // private
}

bool MiscUtils::fileExists(const std::string& path) {
    struct stat exists;
    return stat(path.c_str(), &exists) != -1;
}

}
