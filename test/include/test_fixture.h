/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef TEST_FIXTURE_H_
#define TEST_FIXTURE_H_

#include <gtest/gtest.h>
#include "configcpp/detail/config_base.h"

namespace config {

typedef std::pair<std::string, bool> ParseTest; // parse text, white space matters
typedef std::vector<ParseTest> VectorParseTest;

class TestFixture : public testing::Test {
protected:
    void checkSame(const ConfigBasePtr& a, const ConfigBasePtr& b, const std::string& desc = "");
    void checkNotSame(const ConfigBasePtr& a, const ConfigBasePtr& b, const std::string& desc = "");
    void checkEquals(const ConfigBasePtr& a, const ConfigBasePtr& b, const std::string& desc = "");
    void checkNotEquals(const ConfigBasePtr& a, const ConfigBasePtr& b, const std::string& desc = "");

    void checkNotEqualToRandomOtherThing(const ConfigBasePtr& a, const std::string& desc = "");
    void checkEqualObjects(const ConfigBasePtr& a, const ConfigBasePtr& b, const std::string& desc = "");
    void checkNotEqualObjects(const ConfigBasePtr& a, const ConfigBasePtr& b, const std::string& desc = "");

protected:
    std::string resourcePath();

    /// This is importantly NOT using Path::newPath, which relies on
    /// the parser; in the test suite we are often testing the parser,
    /// so we don't want to use the parser to build the expected result.
    PathPtr path(const VectorString& elements);

    SimpleConfigOriginPtr fakeOrigin();

    VectorParseTest invalidJsonInvalidConf();
    VectorParseTest validJson();
    VectorParseTest validConfInvalidJson();
    VectorParseTest invalidJson();
    VectorParseTest invalidConf();
    VectorParseTest validConf();

    VectorString whitespaceVariations(const VectorParseTest& tests);

    /// It's important that these do NOT use the public API to create the
    /// instances, because we may be testing that the public API returns the
    /// right instance by comparing to these, so using public API here would
    /// make the test compare public API to itself.
    AbstractConfigValuePtr intValue(int32_t i);
    AbstractConfigValuePtr int64Value(int64_t l);
    AbstractConfigValuePtr boolValue(bool b);
    AbstractConfigValuePtr nullValue();
    AbstractConfigValuePtr stringValue(const std::string& s);
    AbstractConfigValuePtr doubleValue(double d);

    AbstractConfigObjectPtr parseObject(const std::string& s);
    ConfigPtr parseConfig(const std::string& s);

    ConfigReferencePtr subst(const std::string& ref, bool optional = false);
    AbstractConfigValuePtr substInString(const std::string& ref, bool optional = false);

    TokenPtr tokenTrue();
    TokenPtr tokenFalse();
    TokenPtr tokenNull();
    TokenPtr tokenUnquoted(const std::string& s);
    TokenPtr tokenString(const std::string& s);
    TokenPtr tokenDouble(double d);
    TokenPtr tokenInt(int32_t i);
    TokenPtr tokenInt64(int64_t i);
    TokenPtr tokenLine(int32_t line);
    TokenPtr tokenComment(const std::string& text);

private:
    TokenPtr tokenMaybeOptionalSubstitution(bool optional, const VectorToken& expression);

protected:
    TokenPtr tokenSubstitution(const VectorToken& expression);
    TokenPtr tokenOptionalSubstitution(const VectorToken& expression);
    TokenPtr tokenKeySubstitution(const std::string& s);

    TokenIteratorPtr tokenize(const ConfigOriginPtr& origin, const ReaderPtr& input);
    TokenIteratorPtr tokenize(const ReaderPtr& input);
    TokenIteratorPtr tokenize(const std::string& s);
    VectorToken tokenizeAsList(const std::string& s);
};

class NotEqualToAnythingElse : public ConfigBase {
public:
    CONFIG_CLASS(NotEqualToAnythingElse);

    virtual bool equals(const ConfigVariant& other) override;
    virtual uint32_t hashCode() override;
};

}

#endif // TEST_FIXTURE_H_
