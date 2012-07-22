/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/path_builder.h"
#include "configcpp/detail/path.h"
#include "configcpp/config_exception.h"

namespace config {

void PathBuilder::checkCanAppend() {
    if (result_) {
        throw ConfigExceptionBugOrBroken("Adding to PathBuilder after getting result");
    }
}

void PathBuilder::appendKey(const std::string& key) {
    checkCanAppend();

    keys.push_front(key);
}

void PathBuilder::appendPath(const PathPtr& path) {
    checkCanAppend();

    std::string first = path->first();
    auto remainder = path->remainder();
    while (true) {
        keys.push_front(first);
        if (remainder) {
            first = remainder->first();
            remainder = remainder->remainder();
        }
        else {
            break;
        }
    }
}

PathPtr PathBuilder::result() {
    // note: if keys is empty, we want to return null, which is a valid empty path
    if (!result_) {
        PathPtr remainder;
        while (!keys.empty()) {
            std::string key = keys.front();
            keys.pop_front();
            remainder = Path::make_instance(key, remainder);
        }
        result_ = remainder;
    }
    return result_;
}

}
