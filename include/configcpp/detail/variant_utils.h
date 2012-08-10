/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_VARIANT_UTILS_H_
#define CONFIG_VARIANT_UTILS_H_

#include "configcpp/config_types.h"

namespace config {

///
/// Collection of boost::variant utility methods.
///

/// Return type from variant
template <typename T>
T variant_get(const ConfigVariant& var) {
    return boost::get<T>(var);
}

/// Return type from variant and static_cast if necessary
template <typename T>
std::shared_ptr<T> static_get(const ConfigVariant& var) {
    return std::static_pointer_cast<T>(boost::get<ConfigBasePtr>(var));
}

/// Return type from variant and dynamic_cast if necessary
template <typename T>
std::shared_ptr<T> dynamic_get(const ConfigVariant& var) {
    return std::dynamic_pointer_cast<T>(boost::get<ConfigBasePtr>(var));
}

struct VariantString : public boost::static_visitor<std::string>
{
    template <typename T>
    std::string operator()(const T& x) const {
        return boost::lexical_cast<std::string>(&x);
    }

    std::string operator()(const null&) const {
        return "null";
    }

    std::string operator()(const int32_t& x) const {
        return boost::lexical_cast<std::string>(x);
    }

    std::string operator()(const int64_t& x) const {
        return boost::lexical_cast<std::string>(x);
    }

    std::string operator()(const double& x) const {
        return boost::lexical_cast<std::string>(x);
    }

    std::string operator()(const bool& x) const {
        return x ? "true" : "false";
    }

    std::string operator()(const std::string& x) const {
        return x;
    }

    std::string operator()(const ConfigBasePtr& x) const {
        return x ? x->toString() : "null";
    }
};

struct VariantInt : public boost::static_visitor<int32_t>
{
    template <typename T>
    int32_t operator()(const T& x) const {
        return 0;
    }

    int32_t operator()(const int32_t& x) const {
        return x;
    }

    int32_t operator()(const int64_t& x) const {
        return static_cast<int32_t>(x);
    }

    int32_t operator()(const double& x) const {
        return static_cast<int32_t>(x);
    }

    int32_t operator()(const bool& x) const {
        return x ? 1 : 0;
    }
};

struct VariantInt64 : public boost::static_visitor<int64_t>
{
    template <typename T>
    int64_t operator()(const T& x) const {
        return 0;
    }

    int64_t operator()(const int32_t& x) const {
        return static_cast<int64_t>(x);
    }

    int64_t operator()(const int64_t& x) const {
        return x;
    }

    int64_t operator()(const double& x) const {
        return static_cast<int64_t>(x);
    }

    int64_t operator()(const bool& x) const {
        return x ? 1 : 0;
    }
};

struct VariantDouble : public boost::static_visitor<double>
{
    template <typename T>
    double operator()(const T& x) const {
        return 0;
    }

    double operator()(const int32_t& x) const {
        return static_cast<double>(x);
    }

    double operator()(const int64_t& x) const {
        return static_cast<double>(x);
    }

    double operator()(const double& x) const {
        return x;
    }

    double operator()(const bool& x) const {
        return x ? 1 : 0;
    }
};

struct VariantHash : public boost::static_visitor<std::size_t>
{
    template <typename T>
    std::size_t operator()(const T& x) const {
        return 0;
    }

    std::size_t operator()(const int32_t& x) const {
        return std::hash<int32_t>()(x);
    }

    std::size_t operator()(const int64_t& x) const {
        return std::hash<int64_t>()(x);
    }

    std::size_t operator()(const double& x) const {
        return std::hash<double>()(x);
    }

    std::size_t operator()(const bool& x) const {
        return std::hash<bool>()(x);
    }

    std::size_t operator()(const std::string& x) const {
        return std::hash<std::string>()(x);
    }

    std::size_t operator()(const ConfigBasePtr& x) const {
        return x ? x->hashCode() : 0;
    }
};

struct VariantEquals : public boost::static_visitor<bool>
{
    VariantEquals(const ConfigVariant& eq) : eq(eq) {
    }

    bool operator()(const null& x) const {
        return instanceof<null>(eq);
    }

    bool operator()(const int32_t& x) const {
        return boost::apply_visitor(VariantInt(), eq) == x;
    }

    bool operator()(const int64_t& x) const {
        return boost::apply_visitor(VariantInt64(), eq) == x;
    }

    bool operator()(const double& x) const {
        return boost::apply_visitor(VariantDouble(), eq) == x;
    }

    bool operator()(const bool& x) const {
        return instanceof<bool>(eq) && variant_get<bool>(eq) == x;
    }

    bool operator()(const std::string& x) const {
        return boost::apply_visitor(VariantString(), eq) == x;
    }

    bool operator()(const ConfigBasePtr& x) const {
        if (!instanceof<ConfigBase>(eq)) {
            return false;
        }
        const ConfigBasePtr& eq_var = variant_get<ConfigBasePtr>(eq);
        if (!eq_var && !x) {
            return true;
        }
        else if ((!eq_var && x) || (eq_var && !x)) {
            return false;
        }
        return eq_var->equals(x);
    }

    bool operator()(const MapVariant& x) const {
        if (!instanceof<MapVariant>(eq)) {
            return false;
        }
        const MapVariant& eq_var = variant_get<MapVariant>(eq);
        if (eq_var.size() != x.size()) {
        }
        for (auto eq_i : eq_var) {
            auto x_i = x.find(eq_i.first);
            if (x_i == x.end() || !boost::apply_visitor(VariantEquals(eq_i.second), x_i->second)) {
                return false;
            }
        }
        return true;
    }

    bool operator()(const VectorVariant& x) const {
        if (!instanceof<VectorVariant>(eq)) {
            return false;
        }
        const VectorVariant& eq_var = variant_get<VectorVariant>(eq);
        if (eq_var.size() != x.size()) {
            return false;
        }
        for (auto eq_i = eq_var.begin(), x_i = x.begin(); eq_i != eq_var.end(); ++eq_i, ++x_i) {
            if (!boost::apply_visitor(VariantEquals(*eq_i), *x_i)) {
                return false;
            }
        }
        return true;
    }

    const ConfigVariant& eq;
};

}

namespace std {

template <> class hash<config::ConfigVariant> {
public:
    std::size_t operator()(const config::ConfigVariant& v) const {
        return boost::apply_visitor(config::VariantHash(), v);
    }
};

}

#endif // CONFIG_VARIANT_UTILS_H_
