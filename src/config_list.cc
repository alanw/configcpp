/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/config_list.h"

namespace config {

VectorConfigValue::const_iterator ConfigList::begin() const {
    return VectorConfigValue::begin();
}

VectorConfigValue::const_iterator ConfigList::end() const {
    return VectorConfigValue::begin();
}

VectorConfigValue::const_reference ConfigList::at(VectorConfigValue::size_type n) const {
    return VectorConfigValue::at(n);
}

VectorConfigValue::const_reference ConfigList::front() const {
    return VectorConfigValue::front();
}

VectorConfigValue::const_reference ConfigList::back() const {
    return VectorConfigValue::back();
}

VectorConfigValue::const_reference ConfigList::operator[](VectorConfigValue::size_type n) const {
    return VectorConfigValue::at(n);
}

bool ConfigList::empty() const {
    return VectorConfigValue::empty();
}

VectorConfigValue::size_type ConfigList::size() const {
    return VectorConfigValue::size();
}

void ConfigList::clear() {
    VectorConfigValue::clear();
}

void ConfigList::pop_back() {
    VectorConfigValue::pop_back();
}

void ConfigList::resize(VectorConfigValue::size_type n, const VectorConfigValue::value_type& val) {
    VectorConfigValue::resize(n, val);
}

VectorConfigValue::const_iterator ConfigList::erase(VectorConfigValue::const_iterator pos) {
    return VectorConfigValue::erase(VectorConfigValue::begin() + std::distance<VectorConfigValue::const_iterator>(begin(), pos));
}

VectorConfigValue::const_iterator ConfigList::insert(VectorConfigValue::const_iterator pos, const VectorConfigValue::value_type& val) {
    return VectorConfigValue::insert(VectorConfigValue::begin() + std::distance<VectorConfigValue::const_iterator>(begin(), pos), val);
}

}
