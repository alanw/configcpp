/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef RESOLVE_STATUS_H_
#define RESOLVE_STATUS_H_

#include "configcpp/config_types.h"

namespace config {

///
/// Status of substitution resolution.
///
enum class ResolveStatus : uint32_t {
    UNRESOLVED, RESOLVED
};

class ResolveStatusEnum {
public:
    static ResolveStatus fromValues(const VectorAbstractConfigValue& values);
    static ResolveStatus fromValues(const MapAbstractConfigValue& values);
    static ResolveStatus fromBool(bool resolved);
};

}

#endif // RESOLVE_STATUS_H_
