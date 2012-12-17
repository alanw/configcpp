/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "test_fixture.h"
#include "configcpp/detail/config_impl_util.h"
#include "configcpp/config.h"

using namespace config;

class UtilTest : public TestFixture {
public:
    void roundtripJson(const std::string& s) {
        auto rendered = ConfigImplUtil::renderJsonString(s);
        auto parsed = parseConfig("{ foo: " + rendered + "}")->getString("foo");
        EXPECT_EQ(s, parsed);
    }

    void roundtripUnquoted(const std::string& s) {
        auto rendered = ConfigImplUtil::renderStringUnquotedIfPossible(s);
        auto parsed = parseConfig("{ foo: " + rendered + "}")->getString("foo");
        EXPECT_EQ(s, parsed);
    }

    VectorString lotsOfStrings() {
        static VectorString _lotsOfStrings;
        if (_lotsOfStrings.empty()) {
            for (auto& s : invalidJson()) {
                _lotsOfStrings.push_back(s.first);
            }
            for (auto& s : validConf()) {
                _lotsOfStrings.push_back(s.first);
            }
        }
        return _lotsOfStrings;
    }

};

TEST_F(UtilTest, renderJsonString) {
    for (auto& s : lotsOfStrings()) {
        roundtripJson(s);
    }
}

TEST_F(UtilTest, renderUnquotedIfPossible) {
    for (auto& s : lotsOfStrings()) {
        roundtripUnquoted(s);
    }
}
