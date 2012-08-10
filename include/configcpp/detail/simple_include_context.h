/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef SIMPLE_INCLUDE_CONTEXT_H_
#define SIMPLE_INCLUDE_CONTEXT_H_

#include "configcpp/detail/config_base.h"
#include "configcpp/config_include_context.h"

namespace config {

class SimpleIncludeContext : public virtual ConfigIncludeContext, public ConfigBase {
public:
    CONFIG_CLASS(SimpleIncludeContext);

    SimpleIncludeContext(const ParseablePtr& parseable);

    SimpleIncludeContextPtr withParseable(const ParseablePtr& parseable);

    virtual ConfigParseablePtr relativeTo(const std::string& filename) override;
    virtual ConfigParseOptionsPtr parseOptions() override;

private:
    ParseablePtr parseable;
};

}

#endif // SIMPLE_INCLUDE_CONTEXT_H_
