/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/token_type.h"

namespace config {

std::string TokenTypeEnum::name(TokenType valueType) {
    typedef std::map<TokenType, std::string> TokenTypeName;
    static TokenTypeName names = {
        {TokenType::START, "START"},
        {TokenType::END, "END"},
        {TokenType::COMMA, "COMMA"},
        {TokenType::EQUALS, "EQUALS"},
        {TokenType::COLON, "COLON"},
        {TokenType::OPEN_CURLY, "OPEN_CURLY"},
        {TokenType::CLOSE_CURLY, "CLOSE_CURLY"},
        {TokenType::OPEN_SQUARE, "OPEN_SQUARE"},
        {TokenType::CLOSE_SQUARE, "CLOSE_SQUARE"},
        {TokenType::VALUE, "VALUE"},
        {TokenType::NEWLINE, "NEWLINE"},
        {TokenType::UNQUOTED_TEXT, "UNQUOTED_TEXT"},
        {TokenType::SUBSTITUTION, "SUBSTITUTION"},
        {TokenType::PROBLEM, "PROBLEM"},
        {TokenType::COMMENT, "COMMENT"},
        {TokenType::PLUS_EQUALS, "PLUS_EQUALS"}
    };
    return names[valueType];
}

}
