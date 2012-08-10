/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef SIMPLE_INCLUDER_H_
#define SIMPLE_INCLUDER_H_

#include "configcpp/detail/config_base.h"
#include "configcpp/detail/full_includer.h"

namespace config {

class SimpleIncluder : public virtual FullIncluder, public ConfigBase {
public:
    CONFIG_CLASS(SimpleIncluder);

    SimpleIncluder(const ConfigIncluderPtr& fallback);

    /// ConfigIncludeContext does this for us on its options
    static ConfigParseOptionsPtr clearForInclude(const ConfigParseOptionsPtr& options);

    /// This is the heuristic includer
    virtual ConfigObjectPtr include(const ConfigIncludeContextPtr& context,
                                    const std::string& what) override;

    /// The heuristic includer in static form
    static ConfigObjectPtr includeWithoutFallback(const ConfigIncludeContextPtr& context,
                                                  const std::string& name);

    virtual ConfigObjectPtr includeFile(const ConfigIncludeContextPtr& context,
                                        const std::string& file) override;

    static ConfigObjectPtr includeFileWithoutFallback(const ConfigIncludeContextPtr& context,
                                                      const std::string& file);

    virtual ConfigIncluderPtr withFallback(const ConfigIncluderPtr& fallback) override;

    /// This function is a little tricky because there are two places we're
    /// trying to use it; for 'include "basename"' in a .conf file, for
    /// loading app.{conf,json} from the filesystem.
    static ConfigObjectPtr fromBasename(const NameSourcePtr& source,
                                        const std::string& name,
                                        const ConfigParseOptionsPtr& options);

    static FullIncluderPtr makeFull(const ConfigIncluderPtr& includer);

private:
    ConfigIncluderPtr fallback;
};

class NameSource {
public:
    virtual ConfigParseablePtr nameToParseable(const std::string& name,
                                               const ConfigParseOptionsPtr& parseOptions) = 0;
};

class RelativeNameSource : public virtual NameSource, public ConfigBase {
public:
    CONFIG_CLASS(RelativeNameSource);

    RelativeNameSource(const ConfigIncludeContextPtr& context);

    virtual ConfigParseablePtr nameToParseable(const std::string& name,
                                               const ConfigParseOptionsPtr& parseOptions) override;

private:
    ConfigIncludeContextPtr context;
};

///
/// The Proxy is a proxy for an application-provided includer that uses our
/// default implementations when the application-provided includer doesn't
/// have an implementation.
///
class FullIncluderProxy : public virtual FullIncluder, public ConfigBase {
public:
    CONFIG_CLASS(FullIncluderProxy);

    FullIncluderProxy(const ConfigIncluderPtr& delegate);

    virtual ConfigIncluderPtr withFallback(const ConfigIncluderPtr& fallback) override;
    virtual ConfigObjectPtr include(const ConfigIncludeContextPtr& context,
                                    const std::string& what) override;
    virtual ConfigObjectPtr includeFile(const ConfigIncludeContextPtr& context,
                                        const std::string& file) override;

public:
    ConfigIncluderPtr delegate;
};

}

#endif // SIMPLE_INCLUDER_H_
