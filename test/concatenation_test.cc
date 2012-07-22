/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "test_fixture.h"
#include "configcpp/config.h"
#include "configcpp/config_list.h"
#include "configcpp/config_object.h"
#include "configcpp/config_exception.h"

using namespace config;

class ConcatenationTest : public TestFixture {
};

TEST_F(ConcatenationTest, noSubstitutionsStringConcat) {
    auto conf = parseConfig(" a :  true \"xyz\" 123 foo  ")->resolve();
    EXPECT_EQ("true xyz 123 foo", conf->getString("a"));
}

TEST_F(ConcatenationTest, trivialStringConcat) {
    auto conf = parseConfig(" a : ${x}foo, x = 1 ")->resolve();
    EXPECT_EQ("1foo", conf->getString("a"));
}

TEST_F(ConcatenationTest, twoSubstitutionsStringConcat) {
    auto conf = parseConfig(" a : ${x}foo${x}, x = 1 ")->resolve();
    EXPECT_EQ("1foo1", conf->getString("a"));
}

TEST_F(ConcatenationTest, stringConcatCannotSpanLines) {
    try {
        parseConfig(" a : ${x}\nfoo, x = 1 ");
        FAIL() << "expected: ConfigExceptionParse";
    }
    catch (ConfigExceptionParse& e) {
        EXPECT_TRUE(boost::contains(e.what(), "not be followed"));
        EXPECT_TRUE(boost::contains(e.what(), "','"));
    }
}

TEST_F(ConcatenationTest, noObjectsInStringConcat) {
    try {
        parseConfig(" a : abc { x : y } ");
        FAIL() << "expected: ConfigExceptionWrongType";
    }
    catch (ConfigExceptionWrongType& e) {
        EXPECT_TRUE(boost::contains(e.what(), "Cannot concatenate"));
        EXPECT_TRUE(boost::contains(e.what(), "abc"));
        EXPECT_TRUE(boost::contains(e.what(), "{\"x\":\"y\"}"));
    }
}

TEST_F(ConcatenationTest, noObjectConcatWithNull) {
    try {
        parseConfig(" a : null { x : y } ");
        FAIL() << "expected: ConfigExceptionWrongType";
    }
    catch (ConfigExceptionWrongType& e) {
        EXPECT_TRUE(boost::contains(e.what(), "Cannot concatenate"));
        EXPECT_TRUE(boost::contains(e.what(), "null"));
        EXPECT_TRUE(boost::contains(e.what(), "{\"x\":\"y\"}"));
    }
}

TEST_F(ConcatenationTest, noArraysInStringConcat) {
    try {
        parseConfig(" a : abc [1, 2] ");
        FAIL() << "expected: ConfigExceptionWrongType";
    }
    catch (ConfigExceptionWrongType& e) {
        EXPECT_TRUE(boost::contains(e.what(), "Cannot concatenate"));
        EXPECT_TRUE(boost::contains(e.what(), "abc"));
        EXPECT_TRUE(boost::contains(e.what(), "[1,2]"));
    }
}

TEST_F(ConcatenationTest, noObjectsSubstitutedInStringConcat) {
    try {
        parseConfig(" a : abc ${x}, x : { y : z } ")->resolve();
        FAIL() << "expected: ConfigExceptionWrongType";
    }
    catch (ConfigExceptionWrongType& e) {
        EXPECT_TRUE(boost::contains(e.what(), "Cannot concatenate"));
        EXPECT_TRUE(boost::contains(e.what(), "abc"));
    }
}

TEST_F(ConcatenationTest, noArraysSubstitutedInStringConcat) {
    try {
        parseConfig(" a : abc ${x}, x : [1,2] ")->resolve();
        FAIL() << "expected: ConfigExceptionWrongType";
    }
    catch (ConfigExceptionWrongType& e) {
        EXPECT_TRUE(boost::contains(e.what(), "Cannot concatenate"));
        EXPECT_TRUE(boost::contains(e.what(), "abc"));
    }
}

TEST_F(ConcatenationTest, noSubstitutionsListConcat) {
    auto conf = parseConfig(" a :  [1,2] [3,4]  ");
    EXPECT_TRUE(VectorVariant({1, 2, 3, 4}) == conf->getList("a")->unwrapped<VectorVariant>());
}

TEST_F(ConcatenationTest, listConcatWithSubstitutions) {
    auto conf = parseConfig(" a :  ${x} [3,4] ${y}, x : [1,2], y : [5,6]  ")->resolve();
    EXPECT_TRUE(VectorVariant({1, 2, 3, 4, 5, 6}) == conf->getList("a")->unwrapped<VectorVariant>());
}

TEST_F(ConcatenationTest, listConcatSelfReferential) {
    auto conf = parseConfig(" a : [1, 2], a : ${a} [3,4], a : ${a} [5,6]  ")->resolve();
    EXPECT_TRUE(VectorVariant({1, 2, 3, 4, 5, 6}) == conf->getList("a")->unwrapped<VectorVariant>());
}

TEST_F(ConcatenationTest, noSubstitutionsListConcatCannotSpanLines) {
    try {
        parseConfig(" a :  [1,2]\n[3,4]  ");
        FAIL() << "expected: ConfigExceptionParse";
    }
    catch (ConfigExceptionParse& e) {
        EXPECT_TRUE(boost::contains(e.what(), "expecting"));
        EXPECT_TRUE(boost::contains(e.what(), "'['"));
    }
}

TEST_F(ConcatenationTest, listConcatCanSpanLinesInsideBrackets) {
    auto conf = parseConfig(" a :  [1,2\n] [3,4]  ");
    ConfigVariant test = conf->getList("a")->unwrapped<VectorVariant>();
    ConfigVariant expected = VectorVariant({1, 2, 3, 4});
    EXPECT_TRUE(boost::apply_visitor(VariantEquals(expected), test));
}

TEST_F(ConcatenationTest, noSubstitutionsObjectConcat) {
    auto conf = parseConfig(" a : { b : c } { x : y }  ");
    ConfigVariant test = conf->getObject("a")->unwrapped<MapVariant>();
    ConfigVariant expected = MapVariant({{"b", std::string("c")}, {"x", std::string("y")}});
    EXPECT_TRUE(boost::apply_visitor(VariantEquals(expected), test));
}

TEST_F(ConcatenationTest, objectConcatMergeOrder) {
    auto conf = parseConfig(" a : { b : 1 } { b : 2 } { b : 3 } { b : 4 } ");
    EXPECT_EQ(4, conf->getInt("a.b"));
}

TEST_F(ConcatenationTest, objectConcatWithSubstitutions) {
    auto conf = parseConfig(" a : ${x} { b : 1 } ${y}, x : { a : 0 }, y : { c : 2 } ")->resolve();
    ConfigVariant test = conf->getObject("a")->unwrapped<MapVariant>();
    ConfigVariant expected = MapVariant({{"a", 0}, {"b", 1}, {"c", 2}});
    EXPECT_TRUE(boost::apply_visitor(VariantEquals(expected), test));
}

TEST_F(ConcatenationTest, objectConcatSelfReferential) {
    auto conf = parseConfig(" a : { a : 0 }, a : ${a} { b : 1 }, a : ${a} { c : 2 } ")->resolve();
    ConfigVariant test = conf->getObject("a")->unwrapped<MapVariant>();
    ConfigVariant expected = MapVariant({{"a", 0}, {"b", 1}, {"c", 2}});
    EXPECT_TRUE(boost::apply_visitor(VariantEquals(expected), test));
}

TEST_F(ConcatenationTest, objectConcatSelfReferentialOverride) {
    auto conf = parseConfig(" a : { b : 3 }, a : { b : 2 } ${a} ")->resolve();
    ConfigVariant test = conf->getObject("a")->unwrapped<MapVariant>();
    ConfigVariant expected = MapVariant({{"b", 3}});
    EXPECT_TRUE(boost::apply_visitor(VariantEquals(expected), test));
}

TEST_F(ConcatenationTest, noSubstitutionsObjectConcatCannotSpanLines) {
    try {
        parseConfig(" a :  { b : c }\n{ x : y }");
        FAIL() << "expected: ConfigExceptionParse";
    }
    catch (ConfigExceptionParse& e) {
        EXPECT_TRUE(boost::contains(e.what(), "expecting"));
        EXPECT_TRUE(boost::contains(e.what(), "'{'"));
    }
}

TEST_F(ConcatenationTest, objectConcatCanSpanLinesInsideBraces) {
    auto conf = parseConfig(" a :  { b : c\n} { x : y }  ");
    ConfigVariant test = conf->getObject("a")->unwrapped<MapVariant>();
    ConfigVariant expected = MapVariant({{"b", std::string("c")}, {"x", std::string("y")}});
    EXPECT_TRUE(boost::apply_visitor(VariantEquals(expected), test));
}

TEST_F(ConcatenationTest, stringConcatInsideArrayValue) {
    auto conf = parseConfig(" a : [ foo bar 10 ] ");
    EXPECT_TRUE(VectorString({"foo bar 10"}) == conf->getStringList("a"));
}

TEST_F(ConcatenationTest, stringNonConcatInsideArrayValue) {
    auto conf = parseConfig(" a : [ foo\n    bar\n     10 ] ");
    EXPECT_TRUE(VectorString({"foo", "bar", "10"}) == conf->getStringList("a"));
}

TEST_F(ConcatenationTest, objectConcatInsideArrayValue) {
    auto conf = parseConfig(" a : [ { b : c } { x : y } ] ");
    VectorVariant unwrapped;
    for (auto& o : conf->getObjectList("a")) {
        unwrapped.push_back(o->unwrapped());
    }
    ConfigVariant test = unwrapped;
    ConfigVariant expected = VectorVariant({MapVariant({{"b", std::string("c")}, {"x", std::string("y")}})});
    EXPECT_TRUE(boost::apply_visitor(VariantEquals(expected), test));
}

TEST_F(ConcatenationTest, objectNonConcatInsideArrayValue) {
    auto conf = parseConfig(" a : [ { b : c }\n  { x : y } ] ");
    VectorVariant unwrapped;
    for (auto& o : conf->getObjectList("a")) {
        unwrapped.push_back(o->unwrapped());
    }
    ConfigVariant test = unwrapped;
    ConfigVariant expected = VectorVariant({MapVariant({{"b", std::string("c")}}), MapVariant({{"x", std::string("y")}})});
    EXPECT_TRUE(boost::apply_visitor(VariantEquals(expected), test));
}

TEST_F(ConcatenationTest, listConcatInsideArrayValue) {
    auto conf = parseConfig(" a : [ [1, 2] [3, 4] ] ");
    ConfigVariant test = conf->getList("a")->unwrapped();
    ConfigVariant expected = VectorVariant({VectorVariant({1, 2, 3, 4})});
    EXPECT_TRUE(boost::apply_visitor(VariantEquals(expected), test));
}

TEST_F(ConcatenationTest, listNonConcatInsideArrayValue) {
    auto conf = parseConfig(" a : [ [1, 2]\n  [3, 4] ] ");
    ConfigVariant test = conf->getList("a")->unwrapped();
    ConfigVariant expected = VectorVariant({VectorVariant({1, 2}), VectorVariant({3, 4})});
    EXPECT_TRUE(boost::apply_visitor(VariantEquals(expected), test));
}

TEST_F(ConcatenationTest, stringConcatsAreKeys) {
    auto conf = parseConfig(" 123 foo : \"value\" ");
    EXPECT_EQ("value", conf->getString("123 foo"));
}

TEST_F(ConcatenationTest, objectsAreNotKeys) {
    try {
        parseConfig("{ { a : 1 } : \"value\" }");
        FAIL() << "expected: ConfigExceptionParse";
    }
    catch (ConfigExceptionParse& e) {
        EXPECT_TRUE(boost::contains(e.what(), "expecting a close"));
        EXPECT_TRUE(boost::contains(e.what(), "'{'"));
    }
}

TEST_F(ConcatenationTest, arraysAreNotKeys) {
    try {
        parseConfig("{ [ \"a\" ] : \"value\" }");
        FAIL() << "expected: ConfigExceptionParse";
    }
    catch (ConfigExceptionParse& e) {
        EXPECT_TRUE(boost::contains(e.what(), "expecting a close"));
        EXPECT_TRUE(boost::contains(e.what(), "'['"));
    }
}

TEST_F(ConcatenationTest, emptyArrayPlusEquals) {
    auto conf = parseConfig(" a = [], a += 2 ")->resolve();
    EXPECT_TRUE(VectorInt({2}) == conf->getIntList("a"));
}

TEST_F(ConcatenationTest, missingArrayPlusEquals) {
    auto conf = parseConfig(" a += 2 ")->resolve();
    EXPECT_TRUE(VectorInt({2}) == conf->getIntList("a"));
}

TEST_F(ConcatenationTest, shortArrayPlusEquals) {
    auto conf = parseConfig(" a = [1], a += 2 ")->resolve();
    EXPECT_TRUE(VectorInt({1, 2}) == conf->getIntList("a"));
}

TEST_F(ConcatenationTest, numberPlusEquals) {
    try {
        parseConfig(" a = 10, a += 2 ")->resolve();
        FAIL() << "expected: ConfigExceptionWrongType";
    }
    catch (ConfigExceptionWrongType& e) {
        EXPECT_TRUE(boost::contains(e.what(), "Cannot concatenate"));
        EXPECT_TRUE(boost::contains(e.what(), "10"));
        EXPECT_TRUE(boost::contains(e.what(), "[2]"));
    }
}

TEST_F(ConcatenationTest, stringPlusEquals) {
    try {
        parseConfig(" a = abc, a += 2 ")->resolve();
        FAIL() << "expected: ConfigExceptionWrongType";
    }
    catch (ConfigExceptionWrongType& e) {
        EXPECT_TRUE(boost::contains(e.what(), "Cannot concatenate"));
        EXPECT_TRUE(boost::contains(e.what(), "abc"));
        EXPECT_TRUE(boost::contains(e.what(), "[2]"));
    }
}

TEST_F(ConcatenationTest, objectPlusEquals) {
    try {
        parseConfig(" a = { x : y }, a += 2 ")->resolve();
        FAIL() << "expected: ConfigExceptionWrongType";
    }
    catch (ConfigExceptionWrongType& e) {
        EXPECT_TRUE(boost::contains(e.what(), "Cannot concatenate"));
        EXPECT_TRUE(boost::contains(e.what(), "\"x\":\"y\""));
        EXPECT_TRUE(boost::contains(e.what(), "[2]"));
    }
}

TEST_F(ConcatenationTest, plusEqualsNestedPath) {
    auto conf = parseConfig(" a.b.c = [1], a.b.c += 2 ")->resolve();
    EXPECT_TRUE(VectorInt({1, 2}) == conf->getIntList("a.b.c"));
}

TEST_F(ConcatenationTest, plusEqualsNestedObjects) {
    auto conf = parseConfig(" a : { b : { c : [1] } }, a : { b : { c += 2 } }")->resolve();
    EXPECT_TRUE(VectorInt({1, 2}) == conf->getIntList("a.b.c"));
}

TEST_F(ConcatenationTest, plusEqualsSingleNestedObject) {
    auto conf = parseConfig(" a : { b : { c : [1], c += 2 } }")->resolve();
    EXPECT_TRUE(VectorInt({1, 2}) == conf->getIntList("a.b.c"));
}

TEST_F(ConcatenationTest, substitutionPlusEqualsSubstitution) {
    auto conf = parseConfig(" a = ${x}, a += ${y}, x = [1], y = 2 ")->resolve();
    EXPECT_TRUE(VectorInt({1, 2}) == conf->getIntList("a"));
}
