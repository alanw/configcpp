/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include <regex>

#include "test_fixture.h"
#include "configcpp/detail/config_reference.h"
#include "configcpp/detail/simple_config_object.h"
#include "configcpp/detail/simple_config.h"
#include "configcpp/config_parse_options.h"
#include "configcpp/config_exception.h"
#include "configcpp/config_mergeable.h"
#include "configcpp/config_origin.h"
#include "configcpp/config.h"

using namespace config;

namespace {

class Problem {
public:
    Problem(const std::string& path, int32_t line) :
        path(path),
        line(line) {
    }

    virtual void check(ValidationProblem& p) {
        EXPECT_EQ(path, p.path());
        EXPECT_EQ(line, p.origin()->lineNumber());
    }

    void expectMessage(ValidationProblem& p, const std::string& re) {
        EXPECT_TRUE(std::regex_match(p.problem(), std::regex(re))) << "Didn't get expected message for " + path + ": got '" + p.problem() + "'";
    }

    std::string path;
    int32_t line;
};

class Missing : public Problem {
public:
    Missing(const std::string& path, int32_t line, const std::string& expected) :
        Problem(path, line),
        expected(expected) {
    }

    virtual void check(ValidationProblem& p) override {
        Problem::check(p);
        expectMessage(p, "No setting.*" + path + ".*expecting.*" + expected + ".*");
    }

    std::string expected;
};

class WrongType : public Problem {
public:
    WrongType(const std::string& path, int32_t line, const std::string& expected, const std::string& got) :
        Problem(path, line),
        expected(expected),
        got(got) {
    }

    virtual void check(ValidationProblem& p) override {
        Problem::check(p);
        expectMessage(p, "List at.*" + path + ".*wrong value type.*expecting.*" + expected + ".*got.*element of.*" + got + ".*");
    }

    std::string expected;
    std::string got;
};

class WrongElementType : public Problem {
public:
    WrongElementType(const std::string& path, int32_t line, const std::string& expected, const std::string& got) :
        Problem(path, line),
        expected(expected),
        got(got) {
    }

    virtual void check(ValidationProblem& p) override {
        Problem::check(p);
        expectMessage(p, "Wrong value type.*" + path + ".*expecting.*" + expected + ".*got.*" + got + ".*");
    }

    std::string expected;
    std::string got;
};

typedef std::vector<Problem> Problems;

}

class ValidationTest : public TestFixture {
protected:
    void checkException(ConfigExceptionValidationFailed& e, Problems& expecteds) {
        VectorValidationProblem problems(e.problems());

        // sort by line number then path
        std::sort(problems.begin(), problems.end(),
            [](const ValidationProblem& first, const ValidationProblem& second) {
                if (const_cast<ValidationProblem&>(first).origin()->lineNumber() < const_cast<ValidationProblem&>(second).origin()->lineNumber()) {
                    return true;
                }
                if (const_cast<ValidationProblem&>(first).origin()->lineNumber() > const_cast<ValidationProblem&>(second).origin()->lineNumber()) {
                    return false;
                }
                return const_cast<ValidationProblem&>(first).path() < const_cast<ValidationProblem&>(second).path();
            }
        );

        ASSERT_EQ(problems.size(), expecteds.size());
        for (uint32_t check = 0; check < problems.size(); ++check) {
            expecteds[check].check(problems[check]);
        }
    }
};

TEST_F(ValidationTest, validation) {
    auto reference = Config::parseFile(resourcePath() + "/validate-reference.conf", ConfigParseOptions::defaults());
    auto conf = Config::parseFile(resourcePath() + "/validate-invalid.conf", ConfigParseOptions::defaults());

    Problems expecteds({
        Missing("willBeMissing", 1, "number"),
        WrongType("int3", 7, "number", "object"),
        WrongType("float2", 9, "number", "boolean"),
        WrongType("float3", 10, "number", "list"),
        WrongType("bool1", 11, "boolean", "number"),
        WrongType("bool3", 13, "boolean", "object"),
        Missing("object1.a", 17, "string"),
        WrongType("object2", 18, "object", "list"),
        WrongType("object3", 19, "object", "number"),
        WrongElementType("array3", 22, "boolean", "object"),
        WrongElementType("array4", 23, "object", "number"),
        WrongType("array5", 24, "list", "number"),
        WrongType("a.b.c.d.e.f.g", 28, "boolean", "number"),
        Missing("a.b.c.d.e.f.j", 28, "boolean"),
        WrongType("a.b.c.d.e.f.i", 30, "boolean", "list")
    });

    try {
        conf->checkValid(reference);
        FAIL() << "expected: ConfigExceptionValidationFailed";
    }
    catch (ConfigExceptionValidationFailed& e) {
        checkException(e, expecteds);
    }
}

TEST_F(ValidationTest, validationWithRoot) {
    auto objectWithB = parseObject("{ b : c }");
    auto reference = Config::parseFile(resourcePath() + "/validate-reference.conf", ConfigParseOptions::defaults())->withFallback(objectWithB);
    auto conf = Config::parseFile(resourcePath() + "/validate-invalid.conf", ConfigParseOptions::defaults());

    Problems expecteds({
        Missing("b", 1, "string"),
        WrongType("a.b.c.d.e.f.g", 28, "boolean", "number"),
        Missing("a.b.c.d.e.f.j", 28, "boolean"),
        WrongType("a.b.c.d.e.f.i", 30, "boolean", "list")
    });

    try {
        conf->checkValid(std::dynamic_pointer_cast<Config>(reference), VectorString({"a", "b"}));
        FAIL() << "expected: ConfigExceptionValidationFailed";
    }
    catch (ConfigExceptionValidationFailed& e) {
        checkException(e, expecteds);
    }
}

TEST_F(ValidationTest, validationCatchesUnresolved) {
    auto reference = parseConfig("{ a : 2 }");
    auto conf = parseConfig("{ b : ${c}, c : 42 }");

    try {
        conf->checkValid(reference);
        FAIL() << "expected: ConfigExceptionNotResolved";
    }
    catch (ConfigExceptionNotResolved& e) {
        EXPECT_TRUE(boost::contains(e.what(), "resolve")) << "Exception message:" << e.what();
    }
}
