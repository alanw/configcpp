/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/config_base.h"
#include "configcpp/config_object.h"

namespace config {

SetConfigValue ConfigObject::entrySet() {
    return SetConfigValue(begin(), end());
}

MapConfigValue::const_iterator ConfigObject::begin() const {
    return MapConfigValue::begin();
}

MapConfigValue::const_iterator ConfigObject::end() const {
    return MapConfigValue::end();
}

MapConfigValue::mapped_type ConfigObject::operator[](const MapConfigValue::key_type& key) const {
    auto val = MapConfigValue::find(key);
    return val == MapConfigValue::end() ? nullptr : val->second;
}

bool ConfigObject::empty() const {
    return MapConfigValue::empty();
}

MapConfigValue::size_type ConfigObject::size() const {
    return MapConfigValue::size();
}

MapConfigValue::size_type ConfigObject::count(const MapConfigValue::key_type& key) const {
    return MapConfigValue::count(key);
}

MapConfigValue::const_iterator ConfigObject::find(const MapConfigValue::key_type& key) const {
    return MapConfigValue::find(key);
}

void ConfigObject::clear() {
    MapConfigValue::clear();
}

MapConfigValue::size_type ConfigObject::erase(const MapConfigValue::key_type& key) {
    return MapConfigValue::erase(key);
}

void ConfigObject::erase(const MapConfigValue::const_iterator& pos) {
    MapConfigValue::erase(pos);
}

void ConfigObject::insert(const MapConfigValue::value_type& val) {
    MapConfigValue::insert(val);
}

}

