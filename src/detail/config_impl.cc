/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/config_impl.h"
#include "configcpp/detail/parseable.h"
#include "configcpp/detail/simple_includer.h"
#include "configcpp/detail/simple_config_origin.h"
#include "configcpp/detail/simple_config_object.h"
#include "configcpp/detail/simple_config_list.h"
#include "configcpp/detail/abstract_config_object.h"
#include "configcpp/detail/from_map_mode.h"
#include "configcpp/detail/config_boolean.h"
#include "configcpp/detail/config_null.h"
#include "configcpp/detail/config_string.h"
#include "configcpp/detail/config_int.h"
#include "configcpp/detail/config_int64.h"
#include "configcpp/detail/config_double.h"
#include "configcpp/detail/path.h"

#ifndef WIN32
#if !HAVE_DECL_ENVIRON
#ifdef __APPLE__
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#else
extern char **environ;
#endif
#endif
#endif

namespace config {

ConfigParseablePtr FileNameSource::nameToParseable(const std::string& name, const ConfigParseOptionsPtr& parseOptions) {
    return Parseable::newFile(name, parseOptions);
}

ConfigObjectPtr ConfigImpl::parseFileAnySyntax(const std::string& basename, const ConfigParseOptionsPtr& baseOptions) {
    auto source = FileNameSource::make_instance();
    return SimpleIncluder::fromBasename(source, basename, baseOptions);
}

AbstractConfigObjectPtr ConfigImpl::emptyObject(const std::string& originDescription) {
    auto origin = !originDescription.empty() ? SimpleConfigOrigin::newSimple(originDescription) : nullptr;
    return emptyObject(origin);
}

ConfigPtr ConfigImpl::emptyConfig(const std::string& originDescription) {
    return emptyObject(originDescription)->toConfig();
}

ConfigOriginPtr ConfigImpl::defaultValueOrigin() {
    static auto defaultValueOrigin_ = SimpleConfigOrigin::newSimple("hardcoded value");
    return defaultValueOrigin_;
}

AbstractConfigObjectPtr ConfigImpl::emptyObject(const ConfigOriginPtr& origin) {
    static auto defaultEmptyObject = SimpleConfigObject::makeEmpty(defaultValueOrigin());

    // we want null origin to go to SimpleConfigObject::makeEmpty() to get the
    // origin "empty config" rather than "hardcoded value"
    if (origin == defaultValueOrigin()) {
        return defaultEmptyObject;
    }
    else {
        return SimpleConfigObject::makeEmpty(origin);
    }
}

SimpleConfigListPtr ConfigImpl::emptyList(const ConfigOriginPtr& origin) {
    static auto defaultEmptyList = SimpleConfigList::make_instance(defaultValueOrigin(), VectorAbstractConfigValue());
    if (!origin || origin == defaultValueOrigin()) {
        return defaultEmptyList;
    }
    else {
        return SimpleConfigList::make_instance(origin, VectorAbstractConfigValue());
    }
}

ConfigOriginPtr ConfigImpl::valueOrigin(const std::string& originDescription) {
    if (originDescription.empty()) {
        return defaultValueOrigin();
    }
    else {
        return SimpleConfigOrigin::newSimple(originDescription);
    }
}

ConfigValuePtr ConfigImpl::fromAnyRef(const ConfigVariant& object, const std::string& originDescription) {
    auto origin = valueOrigin(originDescription);
    return std::dynamic_pointer_cast<ConfigValue>(fromAnyRef(object, origin, FromMapMode::KEYS_ARE_KEYS));
}

ConfigObjectPtr ConfigImpl::fromPathMap(const MapVariant& pathMap, const std::string& originDescription) {
    auto origin = valueOrigin(originDescription);
    return std::dynamic_pointer_cast<ConfigObject>(fromAnyRef(pathMap, origin, FromMapMode::KEYS_ARE_PATHS));
}

AbstractConfigValuePtr ConfigImpl::fromAnyRef(const ConfigVariant& object, const ConfigOriginPtr& origin, FromMapMode mapMode) {
    static auto defaultTrueValue = ConfigBoolean::make_instance(defaultValueOrigin(), true);
    static auto defaultFalseValue = ConfigBoolean::make_instance(defaultValueOrigin(), false);
    static auto defaultNullValue = ConfigNull::make_instance(defaultValueOrigin());

    if (!origin) {
        throw ConfigExceptionBugOrBroken("origin not supposed to be null");
    }
    if (instanceof<null>(object)) {
        if (origin != defaultValueOrigin()) {
            return ConfigNull::make_instance(origin);
        }
        else {
            return defaultNullValue;
        }
    }
    if (instanceof<bool>(object)) {
        if (origin != defaultValueOrigin()) {
            return ConfigBoolean::make_instance(origin, variant_get<bool>(object));
        }
        else if (variant_get<bool>(object)) {
            return defaultTrueValue;
        }
        else {
            return defaultFalseValue;
        }
    }
    if (instanceof<std::string>(object)) {
        return ConfigString::make_instance(origin, variant_get<std::string>(object));
    }
    else if (instanceof<int32_t>(object)) {
        return ConfigInt::make_instance(origin, variant_get<int32_t>(object), "");
    }
    else if (instanceof<int64_t>(object)) {
        return ConfigInt64::make_instance(origin, variant_get<int64_t>(object), "");
    }
    else if (instanceof<double>(object)) {
        return ConfigDouble::make_instance(origin, variant_get<double>(object), "");
    }
    else if (instanceof<MapVariant>(object)) {
        if (variant_get<MapVariant>(object).empty()) {
            return emptyObject(origin);
        }

        if (mapMode == FromMapMode::KEYS_ARE_KEYS) {
            MapAbstractConfigValue values;
            for (auto& entry : variant_get<MapVariant>(object)) {
                auto value = fromAnyRef(entry.second, origin, mapMode);
                values[entry.first] = value;
            }
            return SimpleConfigObject::make_instance(origin, values);
        }
        else {
            return fromPathMap(origin, variant_get<MapVariant>(object));
        }
    }
    else if (instanceof<VectorVariant>(object)) {
        if (variant_get<VectorVariant>(object).empty()) {
            return emptyList(origin);
        }

        VectorAbstractConfigValue values;
        for (auto& i : variant_get<VectorVariant>(object)) {
            auto v = fromAnyRef(i, origin, mapMode);
            values.push_back(v);
        }

        return SimpleConfigList::make_instance(origin, values);
    }
    else {
        std::ostringstream stream;
        stream << "bug in method caller: not valid to create ConfigValue from: ";
        stream << boost::apply_visitor(VariantString(), object);
        throw ConfigExceptionBugOrBroken(stream.str());
    }
}

AbstractConfigObjectPtr ConfigImpl::fromPathMap(const ConfigOriginPtr& origin, const MapVariant& pathExpressionMap) {
    MapPathVariant pathMap;
    for (auto& entry : pathExpressionMap) {
        auto path = Path::newPath(entry.first);
        pathMap[path] = entry.second;
    }

    // First, build a list of paths that will have values, either string or
    // object values.
    SetPath scopePaths;
    SetPath valuePaths;
    for (auto& path : pathMap) {
        // add value's path
        valuePaths.insert(path.first);

        // all parent paths are objects
        auto next = path.first->parent();
        while (next) {
            scopePaths.insert(next);
            next = next->parent();
        }
    }

    for (auto& path : valuePaths) {
        if (scopePaths.find(path) != scopePaths.end()) {
            throw ConfigExceptionBugOrBroken("In the map, path '" + path->render() +
                                             "' occurs as both the parent object of a value and as a value. "
                                             "Because Map has no defined ordering, this is a broken situation.");
        }
    }

    // Create maps for the object-valued values.
    MapAbstractConfigValue root;
    MapPathMapAbstractConfigValue scopes;

    for (auto& path : scopePaths) {
        scopes[path] = MapAbstractConfigValue();
    }

    // Store string values in the associated scope maps
    for (auto& path : valuePaths) {
        auto parentPath = path->parent();
        MapAbstractConfigValue& parent = parentPath ? scopes[parentPath] : root;
        std::string last = path->last();
        ConfigVariant rawValue = pathMap[path];
        auto value = ConfigImpl::fromAnyRef(pathMap[path], origin, FromMapMode::KEYS_ARE_PATHS);
        parent[last] = value;
    }

    // Make a list of scope paths from longest to shortest, so children go
    // before parents.
    VectorPath sortedScopePaths(scopePaths.begin(), scopePaths.end());
    // sort descending by length
    std::sort(sortedScopePaths.begin(), sortedScopePaths.end(),
        [](const PathPtr& first, const PathPtr& second) {
            return second->length() < first->length();
        }
    );

    /// Create ConfigObject for each scope map, working from children to
    /// parents to avoid modifying any already-created ConfigObject. This is
    /// where we need the sorted list.

    for (auto& scopePath : sortedScopePaths) {
        auto parentPath = scopePath->parent();
        auto& parent = parentPath ? scopes[parentPath] : root;
        auto o = SimpleConfigObject::make_instance(origin, scopes[scopePath],
                                                                      ResolveStatus::RESOLVED, false);
        parent[scopePath->last()] = o;
    }

    // return root config object
    return SimpleConfigObject::make_instance(origin, root, ResolveStatus::RESOLVED, false);
}

ConfigIncluderPtr ConfigImpl::defaultIncluder() {
    static auto _defaultIncluder = SimpleIncluder::make_instance(nullptr);
    return _defaultIncluder;
}

MapString ConfigImpl::loadEnvVariables() {
    static MapString env;
    if (env.empty()) {
        VectorString envList;
        #ifdef WIN32
        LPCCH envStrings = GetEnvironmentStrings();
        LPCSTR var = (LPTSTR)envStrings;
        while (*var) {
            envList.push_back(var);
            var += strlen(var) + 1;
        }
        FreeEnvironmentStrings(envStrings);
        #else
        for (uint32_t n = 0; environ[n]; ++n) {
            envList.push_back(environ[n]);
        }
        #endif
        VectorString kv;
        for (auto& envPair : envList) {
            boost::split(kv, envPair, boost::is_any_of("="));
            if (kv.size() == 2) {
                env[kv[0]] = kv[1];
            }
        }
    }
    return env;
}

ConfigPtr ConfigImpl::envVariablesAsConfig() {
    return envVariablesAsConfigObject()->toConfig();
}

AbstractConfigObjectPtr ConfigImpl::envVariablesAsConfigObject() {
    MapString env = loadEnvVariables();
    MapAbstractConfigValue m;
    for (auto& entry : env) {
        m[entry.first] = ConfigString::make_instance(SimpleConfigOrigin::newSimple("env var " + entry.first), entry.second);
    }
    return SimpleConfigObject::make_instance(SimpleConfigOrigin::newSimple("env variables"), m, ResolveStatus::RESOLVED, false);
}

bool ConfigImpl::traceLoadsEnabled() {
    MapString env = loadEnvVariables();
    return env.find("config.trace") != env.end();
}

void ConfigImpl::trace(const std::string& message) {
    std::cerr << message << std::endl;
}

ConfigExceptionNotResolved ConfigImpl::improveNotResolved(const PathPtr& what, const ConfigExceptionNotResolved& original) {
    std::string newMessage = what->render() +
                             " has not been resolved, you need to call Config::resolve()," +
                             " see API docs for Config#resolve()";
    if (newMessage == original.what()) {
        return original;
    }
    else {
        return ConfigExceptionNotResolved(newMessage);
    }
}

}
