/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef TOKEN_TYPE_H_
#define TOKEN_TYPE_H_

#include "configcpp/config_types.h"

namespace config {

enum class TokenType : uint32_t {
    START,
    END,
    COMMA,
    EQUALS,
    COLON,
    OPEN_CURLY,
    CLOSE_CURLY,
    OPEN_SQUARE,
    CLOSE_SQUARE,
    VALUE,
    NEWLINE,
    UNQUOTED_TEXT,
    SUBSTITUTION,
    PROBLEM,
    COMMENT,
    PLUS_EQUALS
};

class TokenTypeEnum {
public:
    static std::string name(TokenType tokenType);
};

}

#endif // TOKEN_TYPE_H_
