/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/simple_includer.h"
#include "configcpp/detail/parseable.h"
#include "configcpp/detail/simple_config_object.h"
#include "configcpp/detail/simple_config_origin.h"
#include "configcpp/config_includer_file.h"
#include "configcpp/config.h"
#include "configcpp/config_syntax.h"
#include "configcpp/config_parseable.h"
#include "configcpp/config_include_context.h"
#include "configcpp/config_object.h"
#include "configcpp/config_parse_options.h"
#include "configcpp/config_exception.h"

namespace config {

SimpleIncluder::SimpleIncluder(const ConfigIncluderPtr& fallback) :
    fallback(fallback) {
}

ConfigParseOptionsPtr SimpleIncluder::clearForInclude(const ConfigParseOptionsPtr& options) {
    // the class loader and includer are inherited, but not this other stuff.
    return options->setSyntax(ConfigSyntax::NONE)->setOriginDescription("")->setAllowMissing(true);
}

ConfigObjectPtr SimpleIncluder::include(const ConfigIncludeContextPtr& context, const std::string& name) {
    auto obj = includeWithoutFallback(context, name);

    // now use the fallback includer if any and merge its result.
    if (fallback) {
        return std::dynamic_pointer_cast<ConfigObject>(obj->withFallback(fallback->include(context, name)));
    }
    else {
        return obj;
    }
}

ConfigObjectPtr SimpleIncluder::includeWithoutFallback(const ConfigIncludeContextPtr& context, const std::string& name) {
    auto source = RelativeNameSource::make_instance(context);
    return fromBasename(source, name, context->parseOptions());
}

ConfigObjectPtr SimpleIncluder::includeFile(const ConfigIncludeContextPtr& context, const std::string& file) {
    auto obj = includeFileWithoutFallback(context, file);

    // now use the fallback includer if any and merge its result.
    if (fallback && instanceof<ConfigIncluderFile>(fallback)) {
        return std::dynamic_pointer_cast<ConfigObject>(obj->withFallback(std::dynamic_pointer_cast<ConfigIncluderFile>(fallback)->includeFile(context, file)));
    }
    else {
        return obj;
    }
}

ConfigObjectPtr SimpleIncluder::includeFileWithoutFallback(const ConfigIncludeContextPtr& context, const std::string& file) {
    return Config::parseFileAnySyntax(file, context->parseOptions())->root();
}

ConfigIncluderPtr SimpleIncluder::withFallback(const ConfigIncluderPtr& fallback) {
    if (shared_from_this() == fallback) {
        throw ConfigExceptionBugOrBroken("trying to create includer cycle");
    }
    else if (this->fallback == fallback) {
        return shared_from_this();
    }
    else if (this->fallback) {
        return make_instance(this->fallback->withFallback(fallback));
    }
    else {
        return make_instance(fallback);
    }
}

ConfigObjectPtr SimpleIncluder::fromBasename(const NameSourcePtr& source, const std::string& name, const ConfigParseOptionsPtr& options) {
    ConfigObjectPtr obj;
    if (boost::ends_with(name, ".conf") || boost::ends_with(name, ".json")) {
        auto p = source->nameToParseable(name, options);
        obj = p->parse(p->options()->setAllowMissing(options->getAllowMissing()));
    }
    else {
        auto confHandle = source->nameToParseable(name + ".conf", options);
        auto jsonHandle = source->nameToParseable(name + ".json", options);
        bool gotSomething = false;
        VectorString failMessages;

        ConfigSyntax syntax = options->getSyntax();

        obj = SimpleConfigObject::makeEmpty(SimpleConfigOrigin::newSimple(name));
        if (syntax == ConfigSyntax::NONE || syntax == ConfigSyntax::CONF) {
            try {
                obj = confHandle->parse(confHandle->options()->setAllowMissing(false)->setSyntax(ConfigSyntax::CONF));
                gotSomething = true;
            }
            catch (ConfigExceptionIO& e) {
                failMessages.push_back(e.what());
            }
        }

        if (syntax == ConfigSyntax::NONE || syntax == ConfigSyntax::JSON) {
            try {
                auto parsed = jsonHandle->parse(jsonHandle->options()->setAllowMissing(false)->setSyntax(ConfigSyntax::JSON));
                obj = std::dynamic_pointer_cast<ConfigObject>(obj->withFallback(parsed));
                gotSomething = true;
            }
            catch (ConfigExceptionIO& e) {
                failMessages.push_back(e.what());
            }
        }

        if (!options->getAllowMissing() && !gotSomething) {
            std::string failMessage;
            if (failMessages.empty()) {
                // this should not happen
                throw ConfigExceptionBugOrBroken("should not be reached: nothing found but no exceptions thrown");
            }
            else {
                std::ostringstream stream;
                for (std::string msg : failMessages) {
                    stream << msg << ", ";
                }
                failMessage = stream.str();
                failMessage = failMessage.substr(0, failMessage.length() - 2);
            }
            throw ConfigExceptionIO(SimpleConfigOrigin::newSimple(name), failMessage);
        }
    }

    return obj;
}

FullIncluderPtr SimpleIncluder::makeFull(const ConfigIncluderPtr& includer) {
    if (instanceof<FullIncluder>(includer)) {
        return std::dynamic_pointer_cast<FullIncluder>(includer);
    }
    else {
        return FullIncluderProxy::make_instance(includer);
    }
}

RelativeNameSource::RelativeNameSource(const ConfigIncludeContextPtr& context) :
    context(context) {
}

ConfigParseablePtr RelativeNameSource::nameToParseable(const std::string& name, const ConfigParseOptionsPtr& options) {
    auto p = context->relativeTo(name);
    if (!p) {
        // avoid returning null
        return Parseable::newNotFound(name, "include was not found: '" + name + "'", options);
    }
    else {
        return p;
    }
}

FullIncluderProxy::FullIncluderProxy(const ConfigIncluderPtr& delegate) :
    delegate(delegate) {
}

ConfigIncluderPtr FullIncluderProxy::withFallback(const ConfigIncluderPtr& fallback) {
    // we never fall back
    return shared_from_this();
}

ConfigObjectPtr FullIncluderProxy::include(const ConfigIncludeContextPtr& context, const std::string& what) {
    return delegate->include(context, what);
}

ConfigObjectPtr FullIncluderProxy::includeFile(const ConfigIncludeContextPtr& context, const std::string& file) {
    if (instanceof<ConfigIncluderFile>(delegate)) {
        return std::dynamic_pointer_cast<ConfigIncluderFile>(delegate)->includeFile(context, file);
    }
    else {
        return SimpleIncluder::includeFileWithoutFallback(context, file);
    }
}

}
