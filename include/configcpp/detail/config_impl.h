/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_IMPL_H_
#define CONFIG_IMPL_H_

#include "configcpp/detail/config_base.h"
#include "configcpp/detail/simple_includer.h"
#include "configcpp/config_exception.h"

namespace config {

class ConfigImpl {
public:
    /// For use ONLY by library internals, DO NOT TOUCH not guaranteed ABI
    static ConfigObjectPtr parseFileAnySyntax(const std::string& basename,
                                              const ConfigParseOptionsPtr& baseOptions);

    static AbstractConfigObjectPtr emptyObject(const std::string& originDescription);

    /// For use ONLY by library internals, DO NOT TOUCH not guaranteed ABI
    static ConfigPtr emptyConfig(const std::string& originDescription);

private:
    /// default origin for values created with fromAnyRef and no origin specified
    static ConfigOriginPtr defaultValueOrigin();

public:
    static AbstractConfigObjectPtr emptyObject(const ConfigOriginPtr& origin);

private:
    static SimpleConfigListPtr emptyList(const ConfigOriginPtr& origin);
    static ConfigOriginPtr valueOrigin(const std::string& originDescription);

public:
    /// For use ONLY by library internals, DO NOT TOUCH not guaranteed ABI
    static ConfigValuePtr fromAnyRef(const ConfigVariant& object,
                                     const std::string& originDescription);

    /// For use ONLY by library internals, DO NOT TOUCH not guaranteed ABI
    static ConfigObjectPtr fromPathMap(const MapVariant& pathMap,
                                       const std::string& originDescription);

    static AbstractConfigValuePtr fromAnyRef(const ConfigVariant& object,
                                             const ConfigOriginPtr& origin,
                                             FromMapMode mapMode);

    static AbstractConfigObjectPtr fromPathMap(const ConfigOriginPtr& origin,
                                               const MapVariant& pathExpressionMap);

    static ConfigIncluderPtr defaultIncluder();

private:
    static MapString loadEnvVariables();

public:
    /// For use ONLY by library internals, DO NOT TOUCH not guaranteed ABI
    static ConfigPtr envVariablesAsConfig();

    static AbstractConfigObjectPtr envVariablesAsConfigObject();

    /// For use ONLY by library internals, DO NOT TOUCH not guaranteed ABI
    static bool traceLoadsEnabled();

    static void trace(const std::string& message);

    /// The basic idea here is to add the "what" and have a canonical
    /// toplevel error message. the "original" exception may however have extra
    /// detail about what happened. call this if you have a better "what" than
    /// further down on the stack.
    static ConfigExceptionNotResolved improveNotResolved(const PathPtr& what,
                                                         const ConfigExceptionNotResolved& original);
};

class FileNameSource : public virtual NameSource, public ConfigBase {
public:
    CONFIG_CLASS(FileNameSource);

    virtual ConfigParseablePtr nameToParseable(const std::string& name,
                                               const ConfigParseOptionsPtr& parseOptions) override;
};

}

#endif // CONFIG_IMPL_H_
