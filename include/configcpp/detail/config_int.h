/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_INT_H_
#define CONFIG_INT_H_

#include "configcpp/detail/config_number.h"

namespace config {

class ConfigInt : public ConfigNumber {
public:
    CONFIG_CLASS(ConfigInt);

    ConfigInt(const ConfigOriginPtr& origin, int32_t value, const std::string& originalText);

    virtual ConfigValueType valueType() override;
    virtual ConfigVariant unwrapped() override;
    virtual std::string transformToString() override;

protected:
    virtual int64_t int64Value() override;
    virtual double doubleValue() override;
    virtual AbstractConfigValuePtr newCopy(const ConfigOriginPtr& origin);

private:
    int32_t value;
};

}

#endif // CONFIG_INT_H_
