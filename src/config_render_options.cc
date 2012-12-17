/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/config_render_options.h"

namespace config {

ConfigRenderOptions::ConfigRenderOptions(bool originComments, bool comments,
                                         bool formatted, bool json) :
    originComments(originComments),
    comments(comments),
    formatted(formatted),
    json(json) {
}

ConfigRenderOptionsPtr ConfigRenderOptions::defaults() {
    return make_instance(true, true, true, true);
}

ConfigRenderOptionsPtr ConfigRenderOptions::concise() {
    return make_instance(false, false, false, true);
}

ConfigRenderOptionsPtr ConfigRenderOptions::setComments(bool value) {
    if (value == comments)
        return shared_from_this();
    else
        return make_instance(originComments, value, formatted, json);
}

bool ConfigRenderOptions::getComments() {
    return comments;
}

ConfigRenderOptionsPtr ConfigRenderOptions::setOriginComments(bool value) {
    if (value == originComments)
        return shared_from_this();
    else
        return make_instance(value, comments, formatted, json);
}

bool ConfigRenderOptions::getOriginComments() {
    return originComments;
}

ConfigRenderOptionsPtr ConfigRenderOptions::setFormatted(bool value) {
    if (value == formatted)
        return shared_from_this();
    else
        return make_instance(originComments, comments, value, json);
}

bool ConfigRenderOptions::getFormatted() {
    return formatted;
}

ConfigRenderOptionsPtr ConfigRenderOptions::setJson(bool value) {
    if (value == json)
        return shared_from_this();
    else
        return make_instance(originComments, comments, formatted, value);
}

bool ConfigRenderOptions::getJson() {
    return json;
}

std::string ConfigRenderOptions::toString() {
    std::string s = getClassName() + "(";
    if (originComments) {
        s += "originComments,";
    }
    if (comments) {
        s += "comments,";
    }
    if (formatted) {
        s += "formatted,";
    }
    if (json) {
        s += "json,";
    }
    if (boost::ends_with(s, ",")) {
        s.resize(s.length() - 1);
    }
    s += ")";
    return s;
}

}
