/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "test_fixture.h"
#include "configcpp/detail/tokenizer.h"
#include "configcpp/detail/tokens.h"
#include "configcpp/detail/config_string.h"
#include "configcpp/detail/simple_config_origin.h"
#include "configcpp/detail/abstract_config_value.h"

using namespace config;

class TokenizerTest : public TestFixture {
protected:
    void tokenizerTest(const VectorToken& expected, const std::string& s) {
        VectorToken complete;
        complete.push_back(Tokens::START());
        complete.insert(complete.end(), expected.begin(), expected.end());
        complete.push_back(Tokens::END());
        EXPECT_TRUE(MiscUtils::vector_equals(complete, tokenizeAsList(s))) << "Not equal:" << s;
    }
};

TEST_F(TokenizerTest, tokenizeEmptyString) {
    EXPECT_TRUE(VectorToken({Tokens::START(), Tokens::END()}) == tokenizeAsList(""));
}

TEST_F(TokenizerTest, tokenizeNewlines) {
    EXPECT_TRUE(MiscUtils::vector_equals(VectorToken({Tokens::START(), tokenLine(1), tokenLine(2), Tokens::END()}), tokenizeAsList("\n\n")));
}

TEST_F(TokenizerTest, tokenizeAllTypesNoSpaces) {
    // all token types with no spaces (not sure JSON spec wants this to work,
    // but spec is unclear to me when spaces are required, and banning them
    // is actually extra work).
    VectorToken expected({
        Tokens::START(), Tokens::COMMA(), Tokens::COLON(), Tokens::EQUALS(),
        Tokens::CLOSE_CURLY(), Tokens::OPEN_CURLY(), Tokens::CLOSE_SQUARE(),
        Tokens::OPEN_SQUARE(), Tokens::PLUS_EQUALS(), tokenString("foo"),
        tokenTrue(), tokenDouble(3.14), tokenFalse(), tokenInt64(42),
        tokenNull(), tokenSubstitution(VectorToken({tokenUnquoted("a.b")})),
        tokenOptionalSubstitution(VectorToken({tokenUnquoted("x.y")})),
        tokenKeySubstitution("c.d"), tokenLine(1), Tokens::END()}
    );
    EXPECT_TRUE(MiscUtils::vector_equals(expected, tokenizeAsList(",:=}{][+=\"foo\"true3.14false42null${a.b}${?x.y}${\"c.d\"}\n")));
}

TEST_F(TokenizerTest, tokenizeAllTypesWithSingleSpaces) {
    VectorToken expected({
        Tokens::START(), Tokens::COMMA(), Tokens::COLON(), Tokens::EQUALS(),
        Tokens::CLOSE_CURLY(), Tokens::OPEN_CURLY(), Tokens::CLOSE_SQUARE(),
        Tokens::OPEN_SQUARE(), Tokens::PLUS_EQUALS(), tokenString("foo"),
        tokenUnquoted(" "), tokenInt64(42), tokenUnquoted(" "), tokenTrue(),
        tokenUnquoted(" "), tokenDouble(3.14), tokenUnquoted(" "), tokenFalse(),
        tokenUnquoted(" "), tokenNull(), tokenUnquoted(" "),
        tokenSubstitution(VectorToken({tokenUnquoted("a.b")})),
        tokenUnquoted(" "),
        tokenOptionalSubstitution(VectorToken({tokenUnquoted("x.y")})),
        tokenUnquoted(" "), tokenKeySubstitution("c.d"), tokenLine(1), Tokens::END()
    });
    EXPECT_TRUE(MiscUtils::vector_equals(expected, tokenizeAsList(" , : = } { ] [ += \"foo\" 42 true 3.14 false null ${a.b} ${?x.y} ${\"c.d\"} \n ")));
}

TEST_F(TokenizerTest, tokenizeAllTypesWithMultipleSpaces) {
    VectorToken expected({
        Tokens::START(), Tokens::COMMA(), Tokens::COLON(), Tokens::EQUALS(),
        Tokens::CLOSE_CURLY(), Tokens::OPEN_CURLY(), Tokens::CLOSE_SQUARE(),
        Tokens::OPEN_SQUARE(), Tokens::PLUS_EQUALS(), tokenString("foo"),
        tokenUnquoted("   "), tokenInt64(42), tokenUnquoted("   "), tokenTrue(),
        tokenUnquoted("   "), tokenDouble(3.14), tokenUnquoted("   "), tokenFalse(),
        tokenUnquoted("   "), tokenNull(), tokenUnquoted("   "),
        tokenSubstitution(VectorToken({tokenUnquoted("a.b")})),
        tokenUnquoted("   "),
        tokenOptionalSubstitution(VectorToken({tokenUnquoted("x.y")})),
        tokenUnquoted("   "), tokenKeySubstitution("c.d"), tokenLine(1), Tokens::END()
    });
    EXPECT_TRUE(MiscUtils::vector_equals(expected, tokenizeAsList("   ,   :   =   }   {   ]   [   +=   \"foo\"   42   true   3.14   false   null   ${a.b}   ${?x.y}   ${\"c.d\"}  \n   ")));
}

TEST_F(TokenizerTest, tokenizeTrueAndUnquotedText) {
    VectorToken expected({
        Tokens::START(), tokenTrue(), tokenUnquoted("foo"), Tokens::END()
    });
    EXPECT_TRUE(MiscUtils::vector_equals(expected, tokenizeAsList("truefoo")));
}

TEST_F(TokenizerTest, tokenizeFalseAndUnquotedText) {
    VectorToken expected({
        Tokens::START(), tokenFalse(), tokenUnquoted("foo"), Tokens::END()
    });
    EXPECT_TRUE(MiscUtils::vector_equals(expected, tokenizeAsList("falsefoo")));
}

TEST_F(TokenizerTest, tokenizeNullAndUnquotedText) {
    VectorToken expected({
        Tokens::START(), tokenNull(), tokenUnquoted("foo"), Tokens::END()
    });
    EXPECT_TRUE(MiscUtils::vector_equals(expected, tokenizeAsList("nullfoo")));
}

TEST_F(TokenizerTest, tokenizeUnquotedTextContainingTrue) {
    VectorToken expected({
        Tokens::START(), tokenUnquoted("footrue"), Tokens::END()
    });
    EXPECT_TRUE(MiscUtils::vector_equals(expected, tokenizeAsList("footrue")));
}

TEST_F(TokenizerTest, tokenizeUnquotedTextContainingSpaceTrue) {
    VectorToken expected({
        Tokens::START(), tokenUnquoted("foo"), tokenUnquoted(" "), tokenTrue(), Tokens::END()
    });
    EXPECT_TRUE(MiscUtils::vector_equals(expected, tokenizeAsList("foo true")));
}

TEST_F(TokenizerTest, tokenizeTrueAndSpaceAndUnquotedText) {
    VectorToken expected({
        Tokens::START(), tokenTrue(), tokenUnquoted(" "), tokenUnquoted("foo"), Tokens::END()
    });
    EXPECT_TRUE(MiscUtils::vector_equals(expected, tokenizeAsList("true foo")));
}

TEST_F(TokenizerTest, tokenizeUnquotedTextContainingSlash) {
    tokenizerTest(VectorToken({tokenUnquoted("a/b/c/")}), "a/b/c/");
    tokenizerTest(VectorToken({tokenUnquoted("/")}), "/");
    tokenizerTest(VectorToken({tokenUnquoted("/"), tokenUnquoted(" "), tokenUnquoted("/")}), "/ /");
    tokenizerTest(VectorToken({tokenComment("")}), "//");
}

TEST_F(TokenizerTest, tokenizeUnquotedTextTrimsSpaces) {
    VectorToken expected({
        Tokens::START(), tokenUnquoted("foo"), tokenLine(1), Tokens::END()
    });
    EXPECT_TRUE(MiscUtils::vector_equals(expected, tokenizeAsList("    foo     \n")));
}

TEST_F(TokenizerTest, tokenizeUnquotedTextKeepsInternalSpaces) {
    VectorToken expected({
        Tokens::START(), tokenUnquoted("foo"), tokenUnquoted("  "),
        tokenUnquoted("bar"), tokenUnquoted(" "), tokenUnquoted("baz"),
        tokenLine(1), Tokens::END()
    });
    EXPECT_TRUE(MiscUtils::vector_equals(expected, tokenizeAsList("    foo  bar baz   \n")));
}

TEST_F(TokenizerTest, tokenizeMixedUnquotedQuoted) {
    VectorToken expected({
        Tokens::START(), tokenUnquoted("foo"), tokenString("bar"),
        tokenUnquoted("baz"), tokenLine(1), Tokens::END()
    });
    EXPECT_TRUE(MiscUtils::vector_equals(expected, tokenizeAsList("    foo\"bar\"baz   \n")));
}

TEST_F(TokenizerTest, tokenizerUnescapeStrings) {
    tokenizerTest(VectorToken({Tokens::newValue(ConfigString::make_instance(fakeOrigin(), ""))}), " \"\" ");
    tokenizerTest(VectorToken({Tokens::newValue(ConfigString::make_instance(fakeOrigin(), std::string("\0", 1)))}), " \"\\u0000\" ");
    tokenizerTest(VectorToken({Tokens::newValue(ConfigString::make_instance(fakeOrigin(), "\"\\/\b\f\n\r\t"))}), " \"\\\"\\\\/\\b\\f\\n\\r\\t\" ");
    tokenizerTest(VectorToken({Tokens::newValue(ConfigString::make_instance(fakeOrigin(), "F"))}), "\"\\u0046\"");
    tokenizerTest(VectorToken({Tokens::newValue(ConfigString::make_instance(fakeOrigin(), "FF"))}), "\"\\u0046\\u0046\"");
}

TEST_F(TokenizerTest, tokenizerReturnsProblemOnInvalidStrings) {
    auto nothingafterslash = tokenizeAsList(" \"\\\" "); // nothing after a backslash
    EXPECT_TRUE(std::find_if(nothingafterslash.begin(), nothingafterslash.end(), Tokens::isProblem) != nothingafterslash.end());
    auto noqescape = tokenizeAsList(" \"\\q\" "); // there is no \q escape sequence
    EXPECT_TRUE(std::find_if(noqescape.begin(), noqescape.end(), Tokens::isProblem) != noqescape.end());
    auto tooshort3 = tokenizeAsList("\"\\u123\""); // too short
    EXPECT_TRUE(std::find_if(tooshort3.begin(), tooshort3.end(), Tokens::isProblem) != tooshort3.end());
    auto tooshort2 = tokenizeAsList("\"\\u12\""); // too short
    EXPECT_TRUE(std::find_if(tooshort2.begin(), tooshort2.end(), Tokens::isProblem) != tooshort2.end());
    auto tooshort1 = tokenizeAsList("\"\\u1\""); // too short
    EXPECT_TRUE(std::find_if(tooshort1.begin(), tooshort1.end(), Tokens::isProblem) != tooshort1.end());
    auto tooshort0 = tokenizeAsList("\"\\u\""); // too short
    EXPECT_TRUE(std::find_if(tooshort0.begin(), tooshort0.end(), Tokens::isProblem) != tooshort0.end());
    auto singlequote = tokenizeAsList("\""); // just a single quote
    EXPECT_TRUE(std::find_if(singlequote.begin(), singlequote.end(), Tokens::isProblem) != singlequote.end());
    auto noendquote = tokenizeAsList(" \"abcdefg"); // no end quote
    EXPECT_TRUE(std::find_if(noendquote.begin(), noendquote.end(), Tokens::isProblem) != noendquote.end());
    auto endslash = tokenizeAsList("\\\"\\"); // file ends with a backslash
    EXPECT_TRUE(std::find_if(endslash.begin(), endslash.end(), Tokens::isProblem) != endslash.end());
    auto enddollar = tokenizeAsList("$"); // file ends with a $
    EXPECT_TRUE(std::find_if(enddollar.begin(), enddollar.end(), Tokens::isProblem) != enddollar.end());
    auto enddollarbrace = tokenizeAsList("${"); // file ends with a ${
    EXPECT_TRUE(std::find_if(enddollarbrace.begin(), enddollarbrace.end(), Tokens::isProblem) != enddollarbrace.end());
}

TEST_F(TokenizerTest, tokenizerParseNumbers) {
    tokenizerTest(VectorToken({tokenInt(1)}), "1");
    tokenizerTest(VectorToken({tokenDouble(1.2)}), "1.2");
    tokenizerTest(VectorToken({tokenInt64(1e6)}), "1e6");
    tokenizerTest(VectorToken({tokenDouble(1e-6)}), "1e-6");
    tokenizerTest(VectorToken({tokenDouble(1e-6)}), "1E-6"); // capital E is allowed
    tokenizerTest(VectorToken({tokenInt(-1)}), "-1");
    tokenizerTest(VectorToken({tokenDouble(-1.2)}), "-1.2");
}

TEST_F(TokenizerTest, commentsHandledInVariousContexts) {
    tokenizerTest(VectorToken({tokenString("//bar")}), "\"//bar\"");
    tokenizerTest(VectorToken({tokenString("#bar")}), "\"#bar\"");
    tokenizerTest(VectorToken({tokenUnquoted("bar"), tokenComment("comment")}), "bar//comment");
    tokenizerTest(VectorToken({tokenUnquoted("bar"), tokenComment("comment")}), "bar#comment");
    tokenizerTest(VectorToken({tokenInt(10), tokenComment("comment")}), "10//comment");
    tokenizerTest(VectorToken({tokenInt(10), tokenComment("comment")}), "10#comment");
    tokenizerTest(VectorToken({tokenDouble(3.14), tokenComment("comment")}), "3.14//comment");
    tokenizerTest(VectorToken({tokenDouble(3.14), tokenComment("comment")}), "3.14#comment");
    // be sure we keep the newline
    tokenizerTest(VectorToken({tokenInt(10), tokenComment("comment"), tokenLine(1), tokenInt(12)}), "10//comment\n12");
    tokenizerTest(VectorToken({tokenInt(10), tokenComment("comment"), tokenLine(1), tokenInt(12)}), "10#comment\n12");
}

TEST_F(TokenizerTest, tokenizeReservedChars) {
    for (auto& invalid : "+`^?!@*&\\") {
        if (invalid == '\0') {
            break; // end of invalid list (and \0 is valid)
        }
        auto tokenized = tokenizeAsList(std::string(1, invalid));
        ASSERT_EQ(3, tokenized.size());
        EXPECT_TRUE(Tokens::START()->equals(tokenized[0]));
        EXPECT_TRUE(Tokens::END()->equals(tokenized[2]));
        auto problem = tokenized[1];
        EXPECT_TRUE(Tokens::isProblem(problem)) << "Reserved char is a problem:" << invalid;
        if (invalid == '+') {
            EXPECT_EQ("end of file", Tokens::getProblemWhat(problem));
        }
        else {
            EXPECT_EQ(std::string(1, invalid), Tokens::getProblemWhat(problem));
        }
    }
}
