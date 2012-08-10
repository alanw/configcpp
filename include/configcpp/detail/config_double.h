/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_DOUBLE_H_
#define CONFIG_DOUBLE_H_

#include "configcpp/detail/config_number.h"

namespace config {

class ConfigDouble : public ConfigNumber {
public:
    CONFIG_CLASS(ConfigDouble);

    ConfigDouble(const ConfigOriginPtr& origin, double value, const std::string& originalText);

    virtual ConfigValueType valueType() override;
    virtual ConfigVariant unwrapped() override;
    virtual std::string transformToString() override;

protected:
    virtual int64_t int64Value() override;
    virtual double doubleValue() override;
    virtual AbstractConfigValuePtr newCopy(const ConfigOriginPtr& origin);

private:
    double value;
};

}

#endif // CONFIG_DOUBLE_H_
