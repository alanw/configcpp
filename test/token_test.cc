/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "test_fixture.h"
#include "configcpp/detail/tokens.h"

using namespace config;

class TokenTest : public TestFixture {
};

TEST_F(TokenTest, tokenEquality) {
    // syntax tokens
    checkEqualObjects(Tokens::START(), Tokens::START());
    checkNotEqualObjects(Tokens::START(), Tokens::OPEN_CURLY());

    // int
    checkEqualObjects(tokenInt(42), tokenInt(42));
    checkNotEqualObjects(tokenInt(42), tokenInt(43));

    // int64
    checkEqualObjects(tokenInt64(42), tokenInt64(42));
    checkNotEqualObjects(tokenInt64(42), tokenInt64(43));

    // int and int64 mixed
    checkEqualObjects(tokenInt(42), tokenInt64(42));
    checkNotEqualObjects(tokenInt(42), tokenInt64(43));

    // boolean
    checkEqualObjects(tokenTrue(), tokenTrue());
    checkNotEqualObjects(tokenTrue(), tokenFalse());

    // double
    checkEqualObjects(tokenDouble(3.14), tokenDouble(3.14));
    checkNotEqualObjects(tokenDouble(3.14), tokenDouble(4.14));

    // string
    checkEqualObjects(tokenString("foo"), tokenString("foo"));
    checkNotEqualObjects(tokenString("foo"), tokenString("bar"));

    // unquoted
    checkEqualObjects(tokenUnquoted("foo"), tokenUnquoted("foo"));
    checkNotEqualObjects(tokenUnquoted("foo"), tokenUnquoted("bar"));

    // key subst
    checkEqualObjects(tokenKeySubstitution("foo"), tokenKeySubstitution("foo"));
    checkNotEqualObjects(tokenKeySubstitution("foo"), tokenKeySubstitution("bar"));

    // null
    checkEqualObjects(tokenNull(), tokenNull());

    // newline
    checkEqualObjects(tokenLine(10), tokenLine(10));
    checkNotEqualObjects(tokenLine(10), tokenLine(11));

    // different types are not equal
    checkNotEqualObjects(tokenTrue(), tokenInt(1));
    checkNotEqualObjects(tokenString("true"), tokenTrue());
}

TEST_F(TokenTest, tokenToString) {
    // just be sure toString() doesn't throw, it's for debugging
    // so its exact output doesn't matter a lot
    EXPECT_NO_THROW(tokenTrue()->toString());
    EXPECT_NO_THROW(tokenFalse()->toString());
    EXPECT_NO_THROW(tokenInt(42)->toString());
    EXPECT_NO_THROW(tokenInt64(43)->toString());
    EXPECT_NO_THROW(tokenDouble(3.14)->toString());
    EXPECT_NO_THROW(tokenNull()->toString());
    EXPECT_NO_THROW(tokenUnquoted("foo")->toString());
    EXPECT_NO_THROW(tokenString("bar")->toString());
    EXPECT_NO_THROW(tokenKeySubstitution("a")->toString());
    EXPECT_NO_THROW(tokenLine(10)->toString());
    EXPECT_NO_THROW(Tokens::START()->toString());
    EXPECT_NO_THROW(Tokens::END()->toString());
    EXPECT_NO_THROW(Tokens::COLON()->toString());
}
