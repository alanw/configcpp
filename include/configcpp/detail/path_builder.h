/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef PATH_BUILDER_H_
#define PATH_BUILDER_H_

#include "configcpp/detail/config_base.h"

namespace config {

class PathBuilder : public ConfigBase {
public:
    CONFIG_CLASS(PathBuilder);

private:
    void checkCanAppend();

public:
    void appendKey(const std::string& key);
    void appendPath(const PathPtr& path);
    PathPtr result();

private:
    /// the keys are kept "backward" (top of stack is end of path)
    StackString keys;

    PathPtr result_;
};

}

#endif // PATH_BUILDER_H_
