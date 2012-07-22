/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_STRING_H_
#define CONFIG_STRING_H_

#include "configcpp/detail/abstract_config_value.h"

namespace config {

class ConfigString : public AbstractConfigValue {
public:
    CONFIG_CLASS(ConfigString);

    ConfigString(const ConfigOriginPtr& origin, const std::string& value);

    virtual ConfigValueType valueType() override;
    virtual ConfigVariant unwrapped() override;
    virtual std::string transformToString() override;

protected:
    virtual void render(std::string& s,
                        uint32_t indent,
                        const ConfigRenderOptionsPtr& options);
    virtual AbstractConfigValuePtr newCopy(const ConfigOriginPtr& origin) override;

private:
    std::string value;
};

}

#endif // CONFIG_STRING_H_
