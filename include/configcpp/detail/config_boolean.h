/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_BOOLEAN_H_
#define CONFIG_BOOLEAN_H_

#include "configcpp/detail/abstract_config_value.h"

namespace config {

class ConfigBoolean : public AbstractConfigValue {
public:
    CONFIG_CLASS(ConfigBoolean);

    ConfigBoolean(const ConfigOriginPtr& origin, bool value);

    virtual ConfigValueType valueType() override;
    virtual ConfigVariant unwrapped() override;
    virtual std::string transformToString() override;

protected:
    virtual AbstractConfigValuePtr newCopy(const ConfigOriginPtr& origin) override;

private:
    bool value;
};

}

#endif // CONFIG_BOOLEAN_H_
