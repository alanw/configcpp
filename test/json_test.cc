/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "test_fixture.h"
#include "configcpp/detail/config_impl_util.h"

using namespace config;

class JsonTest : public TestFixture {
};

TEST_F(JsonTest, renderingJsonStrings) {
    EXPECT_EQ("\"abcdefg\"", ConfigImplUtil::renderJsonString("abcdefg"));
    EXPECT_EQ("\"\\\" \\\\ \\n \\b \\f \\r \\t\"", ConfigImplUtil::renderJsonString("\" \\ \n \b \f \r \t"));
    // control characters are escaped. Remember that unicode escapes
    // are weird and happen on the source file before doing other processing.
    EXPECT_EQ("\"\\u001f\"", ConfigImplUtil::renderJsonString("\u001f"));
}
