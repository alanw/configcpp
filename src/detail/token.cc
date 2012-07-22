/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/token.h"
#include "configcpp/detail/token_type.h"
#include "configcpp/detail/variant_utils.h"
#include "configcpp/config_exception.h"
#include "configcpp/config_origin.h"

namespace config {

Token::Token(TokenType tokenType, const ConfigOriginPtr& origin, const std::string& debugString) :
    tokenType_(tokenType),
    debugString_(debugString),
    origin_(origin) {
}

TokenPtr Token::newWithoutOrigin(TokenType tokenType, const std::string& debugString) {
    return make_instance(tokenType, nullptr, debugString);
}

TokenType Token::tokenType() {
    return tokenType_;
}

ConfigOriginPtr Token::origin() {
    // code is only supposed to call origin() on token types that are
    // expected to have an origin.
    if (!origin_) {
        throw ConfigExceptionBugOrBroken("tried to get origin from token that doesn't have one: " + toString());
    }
    return origin_;
}

int32_t Token::lineNumber() {
    if (origin_) {
        return origin_->lineNumber();
    }
    else {
        return -1;
    }
}

std::string Token::toString() {
    if (!debugString_.empty()) {
        return debugString_;
    }
    else {
        return TokenTypeEnum::name(tokenType_);
    }
}

bool Token::canEqual(const ConfigVariant& other) {
    return instanceof<Token>(other);
}

bool Token::equals(const ConfigVariant& other) {
    if (instanceof<Token>(other)) {
        // origin is deliberately left out
        return canEqual(other) && this->tokenType_ == dynamic_get<Token>(other)->tokenType_;
    }
    else {
        return false;
    }
}

uint32_t Token::hashCode() {
    // origin is deliberately left out
    return std::hash<uint32_t>()(static_cast<uint32_t>(tokenType_));
}

}

