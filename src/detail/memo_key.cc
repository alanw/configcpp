/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/memo_key.h"
#include "configcpp/detail/abstract_config_value.h"
#include "configcpp/detail/path.h"

namespace config {

MemoKey::MemoKey(const AbstractConfigValuePtr& value, const PathPtr& restrictToChildOrNull) :
    value(value),
    restrictToChildOrNull(restrictToChildOrNull) {
}

uint32_t MemoKey::hashCode() {
    uint32_t hash = value->ConfigBase::hashCode();
    if (restrictToChildOrNull) {
        return hash + 41 * (41 + restrictToChildOrNull->hashCode());
    }
    else {
        return hash;
    }
}

bool MemoKey::equals(const ConfigVariant& other) {
    if (instanceof<MemoKey>(other)) {
        auto o = static_get<MemoKey>(other);
        if (o->value != this->value) {
            return false;
        }
        else if (o->restrictToChildOrNull == this->restrictToChildOrNull) {
            return true;
        }
        else if (!o->restrictToChildOrNull || !this->restrictToChildOrNull) {
            return false;
        }
        else {
            return o->restrictToChildOrNull->equals(this->restrictToChildOrNull);
        }
    }
    else {
        return false;
    }
}

}

