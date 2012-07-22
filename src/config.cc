/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/config_impl.h"
#include "configcpp/detail/parseable.h"
#include "configcpp/detail/abstract_config_object.h"
#include "configcpp/config.h"
#include "configcpp/config_object.h"
#include "configcpp/config_parse_options.h"
#include "configcpp/config_resolve_options.h"

namespace config {

ConfigPtr Config::load(const std::string& fileBasename) {
    return load(fileBasename, ConfigParseOptions::defaults(), ConfigResolveOptions::defaults());
}

ConfigPtr Config::load(const std::string& fileBasename,
                       const ConfigParseOptionsPtr& parseOptions,
                       const ConfigResolveOptionsPtr& resolveOptions) {
    auto appConfig = parseFileAnySyntax(fileBasename, parseOptions);
    return load(appConfig, resolveOptions);
}

ConfigPtr Config::load(const ConfigPtr& config, const ConfigResolveOptionsPtr& resolveOptions) {
    return std::dynamic_pointer_cast<Config>(defaultOverrides()->withFallback(config))->resolve(resolveOptions);
}

ConfigPtr Config::defaultOverrides() {
    return ConfigImpl::envVariablesAsConfig();
}

ConfigPtr Config::emptyConfig(const std::string& originDescription) {
    return ConfigImpl::emptyConfig(originDescription);
}

ConfigPtr Config::parseReader(const ReaderPtr& reader, const ConfigParseOptionsPtr& options) {
    if (options) {
        return Parseable::newReader(reader, options)->parse()->toConfig();
    }
    else {
        return Parseable::newReader(reader, ConfigParseOptions::defaults())->parse()->toConfig();
    }
}

ConfigPtr Config::parseFile(const std::string& file, const ConfigParseOptionsPtr& options) {
    if (options) {
        return Parseable::newFile(file, options)->parse()->toConfig();
    }
    else {
        return Parseable::newFile(file, ConfigParseOptions::defaults())->parse()->toConfig();
    }
}

ConfigPtr Config::parseFileAnySyntax(const std::string& fileBasename, const ConfigParseOptionsPtr& options) {
    if (options) {
        return ConfigImpl::parseFileAnySyntax(fileBasename, options)->toConfig();
    }
    else {
        return ConfigImpl::parseFileAnySyntax(fileBasename, ConfigParseOptions::defaults())->toConfig();
    }
}

ConfigPtr Config::parseString(const std::string& s, const ConfigParseOptionsPtr& options) {
    if (options) {
        return Parseable::newString(s, options)->parse()->toConfig();
    }
    else {
        return Parseable::newString(s, ConfigParseOptions::defaults())->parse()->toConfig();
    }
}

ConfigPtr Config::parseMap(const MapVariant& values, const std::string& originDescription) {
    return ConfigImpl::fromPathMap(values, originDescription)->toConfig();
}

}
