/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/config_parse_options.h"
#include "configcpp/config_syntax.h"
#include "configcpp/config_includer.h"

namespace config {

ConfigParseOptions::ConfigParseOptions(ConfigSyntax syntax,
                                       const std::string& originDescription,
                                       bool allowMissing,
                                       const ConfigIncluderPtr& includer) :
    syntax(syntax),
    originDescription(originDescription),
    allowMissing(allowMissing),
    includer(includer) {
}

ConfigParseOptionsPtr ConfigParseOptions::defaults() {
    return make_instance(ConfigSyntax::NONE, "", true, nullptr);
}

ConfigParseOptionsPtr ConfigParseOptions::setSyntax(ConfigSyntax syntax) {
    if (this->syntax == syntax) {
        return shared_from_this();
    }
    else {
        return make_instance(syntax, originDescription, this->allowMissing, this->includer);
    }
}

ConfigSyntax ConfigParseOptions::getSyntax() {
    return syntax;
}

ConfigParseOptionsPtr ConfigParseOptions::setOriginDescription(const std::string& originDescription) {
    if (this->originDescription == originDescription) {
        return shared_from_this();
    }
    else {
        return make_instance(this->syntax, originDescription, this->allowMissing, this->includer);
    }
}

std::string ConfigParseOptions::getOriginDescription() {
    return originDescription;
}

ConfigParseOptionsPtr ConfigParseOptions::withFallbackOriginDescription(const std::string& originDescription) {
    if (this->originDescription.empty()) {
        return setOriginDescription(originDescription);
    }
    else {
        return shared_from_this();
    }
}

ConfigParseOptionsPtr ConfigParseOptions::setAllowMissing(bool allowMissing) {
    if (this->allowMissing == allowMissing) {
        return shared_from_this();
    }
    else {
        return make_instance(this->syntax, this->originDescription, allowMissing, this->includer);
    }
}

bool ConfigParseOptions::getAllowMissing() {
    return allowMissing;
}

ConfigParseOptionsPtr ConfigParseOptions::setIncluder(const ConfigIncluderPtr& includer) {
    if (this->includer == includer) {
        return shared_from_this();
    }
    else {
        return make_instance(this->syntax, this->originDescription, this->allowMissing, includer);
    }
}

ConfigParseOptionsPtr ConfigParseOptions::prependIncluder(const ConfigIncluderPtr& includer) {
    if (this->includer == includer) {
        return shared_from_this();
    }
    else if (this->includer) {
        return setIncluder(includer->withFallback(this->includer));
    }
    else {
        return setIncluder(includer);
    }
}

ConfigParseOptionsPtr ConfigParseOptions::appendIncluder(const ConfigIncluderPtr& includer) {
    if (this->includer == includer) {
        return shared_from_this();
    }
    else if (this->includer) {
        return setIncluder(this->includer->withFallback(includer));
    }
    else {
        return setIncluder(includer);
    }
}

ConfigIncluderPtr ConfigParseOptions::getIncluder() {
    return includer;
}

}
