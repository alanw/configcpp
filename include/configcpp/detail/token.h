/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef TOKEN_H_
#define TOKEN_H_

#include "configcpp/detail/config_base.h"

namespace config {

class Token : public ConfigBase {
public:
    CONFIG_CLASS(Token);

    Token(TokenType tokenType,
          const ConfigOriginPtr& origin,
          const std::string& debugString = "");

    /// This is used for singleton tokens like COMMA or OPEN_CURLY
    static TokenPtr newWithoutOrigin(TokenType tokenType,
                                     const std::string& debugString);

    TokenType tokenType();
    ConfigOriginPtr origin();

    int32_t lineNumber();

    virtual std::string toString() override;

protected:
    virtual bool canEqual(const ConfigVariant& other);

public:
    virtual bool equals(const ConfigVariant& other) override;
    virtual uint32_t hashCode() override;

private:
    TokenType tokenType_;
    std::string debugString_;
    ConfigOriginPtr origin_;
};

}

#endif // TOKEN_H_
