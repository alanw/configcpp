/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/config_render_options.h"

namespace config {

ConfigRenderOptions::ConfigRenderOptions(bool originComments, bool comments, bool formatted) :
    originComments(originComments),
    comments(comments),
    formatted(formatted) {
}

ConfigRenderOptionsPtr ConfigRenderOptions::defaults() {
    return make_instance(true, true, true);
}

ConfigRenderOptionsPtr ConfigRenderOptions::concise() {
    return make_instance(false, false, false);
}

ConfigRenderOptionsPtr ConfigRenderOptions::setComments(bool value) {
    if (value == comments)
        return shared_from_this();
    else
        return make_instance(originComments, value, formatted);
}

bool ConfigRenderOptions::getComments() {
    return comments;
}

ConfigRenderOptionsPtr ConfigRenderOptions::setOriginComments(bool value) {
    if (value == originComments)
        return shared_from_this();
    else
        return make_instance(value, comments, formatted);
}

bool ConfigRenderOptions::getOriginComments() {
    return originComments;
}

ConfigRenderOptionsPtr ConfigRenderOptions::setFormatted(bool value) {
    if (value == formatted)
        return shared_from_this();
    else
        return make_instance(originComments, comments, value);
}

bool ConfigRenderOptions::getFormatted() {
    return formatted;
}

}
