/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/config_base.h"
#include "configcpp/detail/config_impl_util.h"
#include "configcpp/detail/path.h"
#include "configcpp/detail/variant_utils.h"

namespace config {

bool ConfigImplUtil::equalsHandlingNull(const ConfigVariant& a, const ConfigVariant& b) {
    return boost::apply_visitor(VariantEquals(a), b);
}

std::string ConfigImplUtil::renderJsonString(const std::string& s) {
    std::ostringstream stream;
    stream << "\"";
    for (auto& c : s) {
        switch (c) {
            case '"':
                stream << "\\\"";
                break;
            case '\\':
                stream << "\\\\";
                break;
            case '\n':
                stream << "\\n";
                break;
            case '\b':
                stream << "\\b";
                break;
            case '\f':
                stream << "\\f";
                break;
            case '\r':
                stream << "\\r";
                break;
            case '\t':
                stream << "\\t";
                break;
            default:
                if (std::iscntrl(c)) {
                    stream << "\\u" << std::hex << std::nouppercase;
                    stream << std::setfill('0') << std::setw(4) << (int32_t)c;
                }
                else {
                    stream << c;
                }
        }
    }
    stream << "\"";
    return stream.str();
}

std::string ConfigImplUtil::joinPath(const VectorString& elements) {
    return Path::make_instance(elements)->render();
}

VectorString ConfigImplUtil::splitPath(const std::string& path) {
    auto p = Path::newPath(path);
    VectorString elements;
    while (p) {
        elements.push_back(p->first());
        p = p->remainder();
    }
    return elements;
}

}
