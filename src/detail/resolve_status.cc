/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/resolve_status.h"
#include "configcpp/detail/abstract_config_value.h"

namespace config {

ResolveStatus ResolveStatusEnum::fromValues(const VectorAbstractConfigValue& values) {
    for (auto& v : values) {
        if (v->resolveStatus() == ResolveStatus::UNRESOLVED) {
            return ResolveStatus::UNRESOLVED;
        }
    }
    return ResolveStatus::RESOLVED;
}

ResolveStatus ResolveStatusEnum::fromValues(const MapAbstractConfigValue& values) {
    for (auto& v : values) {
        if (v.second->resolveStatus() == ResolveStatus::UNRESOLVED) {
            return ResolveStatus::UNRESOLVED;
        }
    }
    return ResolveStatus::RESOLVED;
}

ResolveStatus ResolveStatusEnum::fromBool(bool resolved) {
    return resolved ? ResolveStatus::RESOLVED : ResolveStatus::UNRESOLVED;
}

}
