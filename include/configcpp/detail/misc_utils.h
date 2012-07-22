/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef MISC_UTILS_H_
#define MISC_UTILS_H_

#include "configcpp/config_types.h"

namespace config {

///
/// Contains generic utility methods.
///
class MiscUtils {
private:
    MiscUtils();

public:
    /// Return a new map that is dynamically cast from elements from another
    template <typename T, typename U>
    static T dynamic_map(U source) {
        T newMap;
        for (auto& v : source) {
            newMap.insert(std::make_pair(v.first, std::dynamic_pointer_cast<typename T::mapped_type::element_type>(v.second)));
        }
        return newMap;
    }

    /// Return a new vector that is dynamically cast from elements from another
    template <typename T, typename U>
    static T dynamic_vector(U source) {
        T newVector;
        newVector.reserve(source.size());
        for (auto& v : source) {
            newVector.push_back(std::dynamic_pointer_cast<typename T::value_type::element_type>(v));
        }
        return newVector;
    }

    /// Extract map keys
    template <typename U, typename V>
    static void key_set(U first, U last, V output) {
        for (; first != last; ++first) {
            *(output++) = first->first;
        }
    }

    /// Compare 2 maps (assumes keys are directly comparable)
    template <typename T>
    static bool map_equals(const T& a, const T& b) {
        if (a.size() != b.size()) {
            return false;
        }
        std::set<typename T::key_type> aKeys;
        key_set(a.begin(), a.end(), std::inserter(aKeys, aKeys.end()));
        std::set<typename T::key_type> bKeys;
        key_set(b.begin(), b.end(), std::inserter(bKeys, bKeys.end()));
        if (aKeys != bKeys) {
            return false;
        }
        for (auto& kv : a) {
            if (!configEquals<ConfigBasePtr>()(std::dynamic_pointer_cast<ConfigBase>(kv.second), std::dynamic_pointer_cast<ConfigBase>(b.find(kv.first)->second))) {
                return false;
            }
        }
        return true;
    }

    /// Compare 2 vectors (assumes elements are of type ConfigBase)
    template <typename T>
    static bool vector_equals(const T& a, const T& b) {
        if (a.size() != b.size()) {
            return false;
        }
        for (uint32_t i = 0; i < a.size(); ++i) {
            if (!configEquals<ConfigBasePtr>()(std::dynamic_pointer_cast<ConfigBase>(a[i]), std::dynamic_pointer_cast<ConfigBase>(b[i]))) {
                return false;
            }
        }
        return true;
    }

    /// Return true is given file path exists
    static bool fileExists(const std::string& path);
};

}

#endif // MISC_UTILS_H_
