/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "test_fixture.h"
#include "configcpp/detail/parseable.h"
#include "configcpp/detail/resolve_context.h"
#include "configcpp/detail/abstract_config_object.h"
#include "configcpp/detail/path.h"
#include "configcpp/detail/config_reference.h"
#include "configcpp/detail/substitution_expression.h"
#include "configcpp/detail/string_reader.h"
#include "configcpp/detail/parser.h"
#include "configcpp/config_parse_options.h"
#include "configcpp/config_exception.h"
#include "configcpp/config_syntax.h"
#include "configcpp/config_origin.h"
#include "configcpp/config_list.h"
#include "configcpp/config_resolve_options.h"
#include "configcpp/config.h"

using namespace config;

class ConfigParserTest : public TestFixture {
protected:
    AbstractConfigValuePtr parseWithoutResolving(const std::string& s) {
        auto options = ConfigParseOptions::defaults()->
                       setOriginDescription("test conf string")->
                       setSyntax(ConfigSyntax::CONF);
        return Parseable::newString(s, options)->parseValue();
    }

    AbstractConfigValuePtr parse(const std::string& s) {
        auto tree = parseWithoutResolving(s);

        // resolve substitutions so we can test problems with that, like cycles or
        // interpolating arrays into strings
        if (instanceof<AbstractConfigObject>(tree)) {
            return ResolveContext::resolve(tree, std::dynamic_pointer_cast<AbstractConfigObject>(tree), ConfigResolveOptions::noSystem());
        }

        return tree;
    }

    PathPtr parsePath(const std::string& s) {
        // parse first by wrapping into a whole document and using
        // the regular parser.
        auto tree = parseWithoutResolving("[${" + s + "}]");
        EXPECT_TRUE(instanceof<ConfigList>(tree));
        auto ref = std::dynamic_pointer_cast<ConfigList>(tree)->at(0);
        EXPECT_TRUE(instanceof<ConfigReference>(ref));
        auto path = std::dynamic_pointer_cast<ConfigReference>(ref)->expression()->path();

        // also parse with the standalone path parser and be sure the
        // outcome is the same.
        auto shouldBeSame = Parser::parsePath(s);
        checkEquals(path, shouldBeSame);

        return path;
    }

    void lineNumberTest(uint32_t num, const std::string& text) {
        try {
            parse(text);
            FAIL() << "expected: ConfigException";
        }
        catch (ConfigException& e) {
            EXPECT_TRUE(boost::contains(e.what(), boost::lexical_cast<std::string>(num) + ":"));
        }
    }
};

TEST_F(ConfigParserTest, invalidConfThrows) {
    // be sure we throw
    for (auto& invalid : whitespaceVariations(invalidConf())) {
        EXPECT_THROW(parse(invalid), ConfigException) << "Expected exception for:" << invalid;
    }
}

TEST_F(ConfigParserTest, validConfWorks) {
    // all we're checking here unfortunately is that it doesn't throw.
    // for a more thorough check, use the EquivalentsTest stuff.
    for (auto& valid : whitespaceVariations(validConf())) {
        ASSERT_NO_THROW({
            auto val = parse(valid);
            auto rendered = val->render();
            auto reparsed = parse(rendered);
            checkEquals(val, reparsed);
        }) << "Unexpected exception for:" << valid;
    }
}

TEST_F(ConfigParserTest, pathParsing) {
    checkEquals(path({"a"}), parsePath("a"));
    checkEquals(path({"a", "b"}), parsePath("a.b"));
    checkEquals(path({"a.b"}), parsePath("\"a.b\""));
    checkEquals(path({"a."}), parsePath("\"a.\""));
    checkEquals(path({".b"}), parsePath("\".b\""));
    checkEquals(path({"true"}), parsePath("true"));
    checkEquals(path({"a"}), parsePath(" a "));
    checkEquals(path({"a ", "b"}), parsePath(" a .b"));
    checkEquals(path({"a ", " b"}), parsePath(" a . b"));
    checkEquals(path({"a  b"}), parsePath(" a  b"));
    checkEquals(path({"a", "b.c", "d"}), parsePath("a.\"b.c\".d"));
    checkEquals(path({"3", "14"}), parsePath("3.14"));
    checkEquals(path({"a3", "14"}), parsePath("a3.14"));
    checkEquals(path({""}), parsePath("\"\""));
    checkEquals(path({"a", "", "b"}), parsePath("a.\"\".b"));
    checkEquals(path({"a", ""}), parsePath("a.\"\""));
    checkEquals(path({"", "b"}), parsePath("\"\".b"));
    checkEquals(path({""}), parsePath("\"\"\"\""));
    checkEquals(path({"a", ""}), parsePath("a.\"\"\"\""));
    checkEquals(path({"", "b"}), parsePath("\"\"\"\".b"));
    checkEquals(path({"", "", ""}), parsePath(" \"\".\"\".\"\" "));
    checkEquals(path({"a-c"}), parsePath("a-c"));
    checkEquals(path({"a_c"}), parsePath("a_c"));
    checkEquals(path({"-"}), parsePath("\"-\""));

    // here 10.0 is part of an unquoted string
    checkEquals(path({"foo10", "0"}), parsePath("foo10.0"));
    // here 10.0 is a number that gets value-concatenated
    checkEquals(path({"10", "0foo"}), parsePath("10.0foo"));
    // just a number
    checkEquals(path({"10", "0"}), parsePath("10.0"));

    VectorString invalid_list({
        {""}, {" "}, {"  \n   \n  "}, {"a."}, {".b"}, {"a..b"}, {"a${b}c"}, {"\"\"."}, {".\"\""}
    });

    for (auto& invalid : invalid_list) {
        EXPECT_THROW(parsePath(invalid), ConfigExceptionBadPath) << "Expected exception for:" << invalid;
    }

    // this gets parsed as a number since it starts with '-'
    EXPECT_THROW(parsePath("-"), ConfigExceptionParse);
}

TEST_F(ConfigParserTest, duplicateKeyLastWins) {
    auto obj = parseConfig("{ \"a\" : 10, \"a\" : 11 } ");

    EXPECT_EQ(1, obj->root()->size());
    EXPECT_EQ(11, obj->getInt("a"));
}

TEST_F(ConfigParserTest, duplicateKeyObjectsMerged) {
    auto obj = parseConfig("{ \"a\" : { \"x\" : 1, \"y\" : 2 }, \"a\" : { \"x\" : 42, \"z\" : 100 } }");

    EXPECT_EQ(1, obj->root()->size());
    EXPECT_EQ(3, obj->getObject("a")->size());
    EXPECT_EQ(42, obj->getInt("a.x"));
    EXPECT_EQ(2, obj->getInt("a.y"));
    EXPECT_EQ(100, obj->getInt("a.z"));
}

TEST_F(ConfigParserTest, duplicateKeyObjectsMergedRecursively) {
    auto obj = parseConfig("{ \"a\" : { \"b\" : { \"x\" : 1, \"y\" : 2 } }, \"a\" : { \"b\" : { \"x\" : 42, \"z\" : 100 } } }");

    EXPECT_EQ(1, obj->root()->size());
    EXPECT_EQ(1, obj->getObject("a")->size());
    EXPECT_EQ(3, obj->getObject("a.b")->size());
    EXPECT_EQ(42, obj->getInt("a.b.x"));
    EXPECT_EQ(2, obj->getInt("a.b.y"));
    EXPECT_EQ(100, obj->getInt("a.b.z"));
}

TEST_F(ConfigParserTest, duplicateKeyObjectsMergedRecursivelyDeeper) {
    auto obj = parseConfig("{ \"a\" : { \"b\" : { \"c\" : { \"x\" : 1, \"y\" : 2 } } }, \"a\" : { \"b\" : { \"c\" : { \"x\" : 42, \"z\" : 100 } } } }");

    EXPECT_EQ(1, obj->root()->size());
    EXPECT_EQ(1, obj->getObject("a")->size());
    EXPECT_EQ(1, obj->getObject("a.b")->size());
    EXPECT_EQ(3, obj->getObject("a.b.c")->size());
    EXPECT_EQ(42, obj->getInt("a.b.c.x"));
    EXPECT_EQ(2, obj->getInt("a.b.c.y"));
    EXPECT_EQ(100, obj->getInt("a.b.c.z"));
}

TEST_F(ConfigParserTest, duplicateKeyObjectNullObject) {
    // null is supposed to "reset" the object at key "a"
    auto obj = parseConfig("{ a : { b : 1 }, a : null, a : { c : 2 } }");

    EXPECT_EQ(1, obj->root()->size());
    EXPECT_EQ(1, obj->getObject("a")->size());
    EXPECT_EQ(2, obj->getInt("a.c"));
}

TEST_F(ConfigParserTest, duplicateKeyObjectNumberObject) {
    auto obj = parseConfig("{ a : { b : 1 }, a : 42, a : { c : 2 } }");

    EXPECT_EQ(1, obj->root()->size());
    EXPECT_EQ(1, obj->getObject("a")->size());
    EXPECT_EQ(2, obj->getInt("a.c"));
}

TEST_F(ConfigParserTest, impliedCommaHandling) {
    VectorString valids({
        "\n"
        "// one line\n"
        "{\n"
        "  a : y, b : z, c : [ 1, 2, 3 ]\n"
        "}\n",
        "// multiline but with all commas\n"
        "{\n"
        "  a : y,\n"
        "  b : z,\n"
        "  c : [\n"
        "    1,\n"
        "    2,\n"
        "    3,\n"
        "  ],\n"
        "}\n",
        "// multiline with no commas\n"
        "{\n"
        "  a : y\n"
        "  b : z\n"
        "  c : [\n"
        "    1\n"
        "    2\n"
        "    3\n"
        "  ]\n"
        "}\n"
    });

    VectorString changes;
    for (auto& v : valids) {
        changes.push_back(boost::replace_all_copy(v, "\n", "\n\n"));
        changes.push_back(boost::replace_all_copy(v, "\n", "\n\n\n"));
        changes.push_back(boost::replace_all_copy(v, ",\n", "\n,\n"));
        changes.push_back(boost::replace_all_copy(v, ",\n", "\n\n,\n\n"));
        changes.push_back(boost::replace_all_copy(v, "\n", " \n "));
        changes.push_back(boost::replace_all_copy(v, ",\n", "  \n  \n  ,  \n  \n  "));
        auto first = v.find('{');
        auto last = v.rfind('}');
        changes.push_back(v.substr(0, first) + v.substr(first + 1, last - first - 1) + v.substr(last + 1));
    }

    for (auto& v : changes) {
        auto obj = parseConfig(v);
        EXPECT_EQ(3, obj->root()->size());
        EXPECT_EQ("y", obj->getString("a"));
        EXPECT_EQ("z", obj->getString("b"));
        EXPECT_TRUE(VectorInt({1, 2, 3}) == obj->getIntList("c"));
    }

    // with no newline or comma, we do value concatenation
    auto noNewlineInArray = parseConfig(" { c : [ 1 2 3 ] } ");
    EXPECT_TRUE(VectorString({"1 2 3"}) == noNewlineInArray->getStringList("c"));

    auto noNewlineInArrayWithQuoted = parseConfig(" { c : [ \"4\" \"5\" \"6\" ] } ");
    EXPECT_TRUE(VectorString({"4 5 6"}) == noNewlineInArrayWithQuoted->getStringList("c"));

    auto noNewlineInObject = parseConfig(" { a : b c } ");
    EXPECT_EQ("b c", noNewlineInObject->getString("a"));

    auto noNewlineAtEnd = parseConfig("a : b");
    EXPECT_EQ("b", noNewlineAtEnd->getString("a"));

    EXPECT_THROW(parseConfig("{ a : y b : z }"), ConfigException);
    EXPECT_THROW(parseConfig("{ \"a\" : \"y\" \"b\" : \"z\" }"), ConfigException);
}

TEST_F(ConfigParserTest, keysWithSlash) {
    auto obj = parseConfig("/a/b/c=42, x/y/z : 32");
    EXPECT_EQ(42, obj->getInt("/a/b/c"));
    EXPECT_EQ(32, obj->getInt("x/y/z"));
}

TEST_F(ConfigParserTest, lineNumbersInErrors) {
    // error is at the last char
    lineNumberTest(1, "}");
    lineNumberTest(2, "\n}");
    lineNumberTest(3, "\n\n}");

    // error is before a final newline
    lineNumberTest(1, "}\n");
    lineNumberTest(2, "\n}\n");
    lineNumberTest(3, "\n\n}\n");

    // with unquoted string
    lineNumberTest(1, "foo");
    lineNumberTest(2, "\nfoo");
    lineNumberTest(3, "\n\nfoo");

    // with quoted string
    lineNumberTest(1, "\"foo\"");
    lineNumberTest(2, "\n\"foo\"");
    lineNumberTest(3, "\n\n\"foo\"");

    // newline in middle of number uses the line the number was on
    lineNumberTest(1, "1e\n");
    lineNumberTest(2, "\n1e\n");
    lineNumberTest(3, "\n\n1e\n");
}

TEST_F(ConfigParserTest, toStringForParseables) {
    // just be sure the toString don't throw, to get test coverage
    auto options = ConfigParseOptions::defaults();
    Parseable::newFile("foo", options)->toString();
    Parseable::newReader(StringReader::make_instance("{}"), options)->toString();
}

TEST_F(ConfigParserTest, trackCommentsForFields) {
    // comment in front of a field is used
    auto conf1 = parseConfig(
        "\n"
        "{ # Hello\n"
        "foo=10 }\n"
    );
    EXPECT_TRUE(VectorString({" Hello"}) == conf1->getValue("foo")->origin()->comments());

    // comment with a blank line after is dropped
    auto conf2 = parseConfig(
        "\n"
        "{ # Hello\n"
        "\n"
        "foo=10 }\n"
    );
    EXPECT_TRUE(VectorString() == conf2->getValue("foo")->origin()->comments());

    // comment in front of a field is used with no root {}
    auto conf3 = parseConfig(
        "\n"
        "# Hello\n"
        "foo=10\n"
    );
    EXPECT_TRUE(VectorString({" Hello"}) == conf3->getValue("foo")->origin()->comments());

    // comment with a blank line after is dropped with no root {}
    auto conf4 = parseConfig(
        "\n"
        "# Hello\n"
        "\n"
        "foo=10\n"
    );
    EXPECT_TRUE(VectorString() == conf4->getValue("foo")->origin()->comments());

    // nested objects
    auto conf5 = parseConfig(
        "\n"
        "# Outside\n"
        "bar {\n"
        "  # Ignore me\n"
        "  \n"
        "  \n"
        "  # Middle\n"
        "  # two lines\n"
        "  baz {\n"
        "    # Inner\n"
        "    foo=10 # should be ignored\n"
        "    # This should be ignored too\n"
        "  } ## not used\n"
        "  # ignored\n"
        "}\n"
        "# ignored!\n"
    );
    EXPECT_TRUE(VectorString({" Inner"}) == conf5->getValue("bar.baz.foo")->origin()->comments());
    EXPECT_TRUE(VectorString({" Middle", " two lines"}) == conf5->getValue("bar.baz")->origin()->comments());
    EXPECT_TRUE(VectorString({" Outside"}) == conf5->getValue("bar")->origin()->comments());

    // multiple fields
    auto conf6 = parseConfig(
        "{\n"
        "# this is not with a field\n"
        "\n"
        "# this is field A\n"
        "a : 10\n"
        "# this is field B\n"
        "b : 12 # goes with field C\n"
        "# this is field C\n"
        "c : 14,\n"
        "\n"
        "# this is not used\n"
        "# nor is this\n"
        "# multi-line block\n"
        "\n"
        "# this is with field D\n"
        "# this is with field D also\n"
        "d : 16\n"
        "\n"
        "# this is after the fields\n"
        "}"
    );
    EXPECT_TRUE(VectorString({" this is field A"}) == conf6->getValue("a")->origin()->comments());
    EXPECT_TRUE(VectorString({" this is field B"}) == conf6->getValue("b")->origin()->comments());
    EXPECT_TRUE(VectorString({" goes with field C", " this is field C"}) == conf6->getValue("c")->origin()->comments());
    EXPECT_TRUE(VectorString({" this is with field D", " this is with field D also"}) == conf6->getValue("d")->origin()->comments());

    // array
    auto conf7 = parseConfig(
        "\n"
        "array = [\n"
        "# goes with 0\n"
        "0,\n"
        "\n"
        "# goes with 1\n"
        "1, # with 2\n"
        "# goes with 2\n"
        "2\n"
        "# not with anything\n"
        "]\n"
    );
    EXPECT_TRUE(VectorString({" goes with 0"}) == conf7->getList("array")->at(0)->origin()->comments());
    EXPECT_TRUE(VectorString({" goes with 1"}) == conf7->getList("array")->at(1)->origin()->comments());
    EXPECT_TRUE(VectorString({" with 2", " goes with 2"}) == conf7->getList("array")->at(2)->origin()->comments());

    // properties-like syntax
    auto conf8 = parseConfig(
        "\n"
        "# ignored comment\n"
        "\n"
        "# x.y comment\n"
        "x.y = 10\n"
        "# x.z comment\n"
        "x.z = 11\n"
        "# x.a comment\n"
        "x.a = 12\n"
        "# a.b comment\n"
        "a.b = 14\n"
        "a.c = 15\n"
        "# ignored comment\n"
    );

    EXPECT_TRUE(VectorString({" x.y comment"}) == conf8->getValue("x.y")->origin()->comments());
    EXPECT_TRUE(VectorString({" x.z comment"}) == conf8->getValue("x.z")->origin()->comments());
    EXPECT_TRUE(VectorString({" x.a comment"}) == conf8->getValue("x.a")->origin()->comments());
    EXPECT_TRUE(VectorString({" a.b comment"}) == conf8->getValue("a.b")->origin()->comments());
    EXPECT_TRUE(VectorString() == conf8->getValue("a.c")->origin()->comments());
    // here we're concerned that comments apply only to leaf
    // nodes, not to parent objects.
    EXPECT_TRUE(VectorString() == conf8->getValue("x")->origin()->comments());
    EXPECT_TRUE(VectorString() == conf8->getValue("a")->origin()->comments());
}

TEST_F(ConfigParserTest, includeFile) {
    auto conf = Config::parseString("include file(\"" + resourcePath() + "/test01" + "\")");

    // should have loaded conf, json
    EXPECT_EQ(42, conf->getInt("ints.fortyTwo"));
    EXPECT_EQ(1, conf->getInt("fromJson1"));
}

TEST_F(ConfigParserTest, includeFileWithExtension) {
    auto conf = Config::parseString("include file(\"" + resourcePath() + "/test01.conf" + "\")");

    // should have loaded conf, json
    EXPECT_EQ(42, conf->getInt("ints.fortyTwo"));
    EXPECT_FALSE(conf->hasPath("fromJson1"));
}

TEST_F(ConfigParserTest, includeFileWhitespaceInsideParens) {
    auto conf = Config::parseString("include file(  \n  \"" + resourcePath() + "/test01" + "\"  \n  )");

    // should have loaded conf, json
    EXPECT_EQ(42, conf->getInt("ints.fortyTwo"));
    EXPECT_EQ(1, conf->getInt("fromJson1"));
}

TEST_F(ConfigParserTest, includeFileNoWhitespaceOutsideParens) {
    try {
        Config::parseString("include file (\"" + resourcePath() + "/test01" + "\")");
        FAIL() << "expected: ConfigExceptionParse";
    }
    catch (ConfigExceptionParse& e) {
        EXPECT_TRUE(boost::contains(e.what(), "expecting include parameter"));
    }
}

TEST_F(ConfigParserTest, includeFileNotQuoted) {
    try {
        Config::parseString("include file(" + resourcePath() + "/test01" + ")");
        FAIL() << "expected: ConfigExceptionParse";
    }
    catch (ConfigExceptionParse& e) {
        EXPECT_TRUE(boost::contains(e.what(), "expecting include parameter"));
    }
}

TEST_F(ConfigParserTest, includeFileNotQuotedAndSpecialChar) {
    try {
        Config::parseString("include file(:" + resourcePath() + "/test01" + ")");
        FAIL() << "expected: ConfigExceptionParse";
    }
    catch (ConfigExceptionParse& e) {
        EXPECT_TRUE(boost::contains(e.what(), "expecting a quoted string"));
    }
}

TEST_F(ConfigParserTest, includeFileUnclosedParens) {
    try {
        Config::parseString("include file(\"" + resourcePath() + "/test01" + "\" something");
        FAIL() << "expected: ConfigExceptionParse";
    }
    catch (ConfigExceptionParse& e) {
        EXPECT_TRUE(boost::contains(e.what(), "expecting a close paren"));
    }
}
