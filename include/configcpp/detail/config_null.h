/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_NULL_H_
#define CONFIG_NULL_H_

#include "configcpp/detail/abstract_config_value.h"

namespace config {

///
/// This exists because sometimes null is not the same as missing. Specifically,
/// if a value is set to null we can give a better error message (indicating
/// where it was set to null) in case someone asks for the value. Also, null
/// overrides values set "earlier" in the search path, while missing values do
/// not.
///
class ConfigNull : public AbstractConfigValue {
public:
    CONFIG_CLASS(ConfigNull);

    ConfigNull(const ConfigOriginPtr& origin);

    virtual ConfigValueType valueType() override;
    virtual ConfigVariant unwrapped() override;
    virtual std::string transformToString() override;

protected:
    virtual AbstractConfigValuePtr newCopy(const ConfigOriginPtr& origin) override;
};

}

#endif // CONFIG_NULL_H_
