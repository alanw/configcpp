/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_NUMBER_H_
#define CONFIG_NUMBER_H_

#include "configcpp/detail/abstract_config_value.h"

namespace config {

class ConfigNumber : public AbstractConfigValue {
public:
    CONFIG_CLASS(ConfigNumber);

protected:
    ConfigNumber(const ConfigOriginPtr& origin, const std::string& originalText);

public:
    virtual ConfigVariant unwrapped() = 0;

    /// Alternative to unwrapping the value to a ConfigVariant.
    template <typename T> T unwrapped() {
        return variant_get<T>(unwrapped());
    }

    virtual std::string transformToString() override;

    int32_t intValueRangeChecked(const std::string& path);

    virtual int64_t int64Value() = 0;
    virtual double doubleValue() = 0;

private:
    bool isWhole();

protected:
    virtual bool canEqual(const ConfigVariant& other) override;

public:
    virtual bool equals(const ConfigVariant& other) override;
    virtual uint32_t hashCode() override;

    static ConfigNumberPtr newNumber(const ConfigOriginPtr& origin,
                                     int64_t number,
                                     const std::string& originalText);
    static ConfigNumberPtr newNumber(const ConfigOriginPtr& origin,
                                     double number,
                                     const std::string& originalText);

protected:
    /// This is so when we concatenate a number into a string (say it appears in
    /// a sentence) we always have it exactly as the person typed it into the
    /// config file. It's purely cosmetic; equals/hashCode don't consider this
    /// for example.
    std::string originalText;
};

}

#endif // CONFIG_NUMBER_H_
