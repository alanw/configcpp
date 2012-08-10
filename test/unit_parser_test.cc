/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "test_fixture.h"
#include "configcpp/detail/simple_config.h"
#include "configcpp/detail/simple_config_origin.h"
#include "configcpp/config_exception.h"

using namespace config;

class UnitParserTest : public TestFixture {
};

TEST_F(UnitParserTest, parseDuration) {
    VectorString oneSecs({
        "1s", "1 s", "1seconds", "1 seconds", "   1s    ", "   1    s   ",
        "1second",
        "1000", "1000ms", "1000 ms", "1000   milliseconds", "   1000       milliseconds    ",
        "1000millisecond",
        "1000000us", "1000000   us", "1000000 microseconds", "1000000microsecond",
        "1000000000ns", "1000000000 ns", "1000000000  nanoseconds", "1000000000nanosecond",
        "0.01666666666666666666666m", "0.01666666666666666666666 minutes", "0.01666666666666666666666 minute",
        "0.00027777777777777777777h", "0.00027777777777777777777 hours", "0.00027777777777777777777hour",
        "1.1574074074074073e-05d", "1.1574074074074073e-05  days", "1.1574074074074073e-05day"
    });
    auto oneSecInNanos = 1000000000;
    for (auto& s : oneSecs) {
        auto result = SimpleConfig::parseDuration(s, fakeOrigin(), "test");
        EXPECT_EQ(oneSecInNanos, result);
    }

    // bad units
    try {
        SimpleConfig::parseDuration("100 dollars", fakeOrigin(), "test");
        FAIL() << "expected: ConfigExceptionBadValue";
    }
    catch (ConfigExceptionBadValue& e) {
        EXPECT_TRUE(boost::contains(e.what(), "time unit")) << "Exception message:" << e.what();
    }

    // bad number
    try {
        SimpleConfig::parseDuration("1 00 seconds", fakeOrigin(), "test");
        FAIL() << "expected: ConfigExceptionBadValue";
    }
    catch (ConfigExceptionBadValue& e) {
        EXPECT_TRUE(boost::contains(e.what(), "duration number")) << "Exception message:" << e.what();
    }
}

TEST_F(UnitParserTest, parseMemorySizeInBytes) {
    VectorString oneMebis({
        "1048576", "1048576b", "1048576bytes", "1048576byte",
        "1048576  b", "1048576  bytes",
        "    1048576  b   ", "  1048576  bytes   ",
        "1048576B",
        "1024k", "1024K", "1024Ki", "1024KiB", "1024 kibibytes", "1024 kibibyte",
        "1m", "1M", "1 M", "1Mi", "1MiB", "1 mebibytes", "1 mebibyte",
        "0.0009765625g", "0.0009765625G", "0.0009765625Gi", "0.0009765625GiB", "0.0009765625 gibibytes", "0.0009765625 gibibyte"
    });

    for (auto& s : oneMebis) {
        auto result = SimpleConfig::parseBytes(s, fakeOrigin(), "test");
        EXPECT_EQ(1024 * 1024, result);
    }

    VectorString oneMegas({
        "1000000", "1000000b", "1000000bytes", "1000000byte",
        "1000000  b", "1000000  bytes",
        "    1000000  b   ", "  1000000  bytes   ",
        "1000000B",
        "1000kB", "1000 kilobytes", "1000 kilobyte",
        "1MB", "1 megabytes", "1 megabyte",
        ".001GB", ".001 gigabytes", ".001 gigabyte"
    });

    for (auto& s : oneMegas) {
        auto result = SimpleConfig::parseBytes(s, fakeOrigin(), "test");
        EXPECT_EQ(1000 * 1000, result);
    }

    auto result = 1024LL * 1024 * 1024;
    for (auto& unit : VectorString({"tebi", "pebi", "exbi", "zebi", "yobi"})) {
        auto first = boost::to_upper_copy(unit.substr(0, 1));
        result *= 1024LL;
        EXPECT_EQ(result, SimpleConfig::parseBytes("1" + first, fakeOrigin(), "test"));
        EXPECT_EQ(result, SimpleConfig::parseBytes("1" + first + "i", fakeOrigin(), "test"));
        EXPECT_EQ(result, SimpleConfig::parseBytes("1" + first + "iB", fakeOrigin(), "test"));
        EXPECT_EQ(result, SimpleConfig::parseBytes("1" + unit + "byte", fakeOrigin(), "test"));
        EXPECT_EQ(result, SimpleConfig::parseBytes("1" + unit + "bytes", fakeOrigin(), "test"));
    }

    result = 1000LL * 1000 * 1000;
    for (auto& unit : VectorString({"tera", "peta", "exa", "zetta", "yotta"})) {
        auto first = boost::to_upper_copy(unit.substr(0, 1));
        result *= 1000LL;
        EXPECT_EQ(result, SimpleConfig::parseBytes("1" + first + "B", fakeOrigin(), "test"));
        EXPECT_EQ(result, SimpleConfig::parseBytes("1" + unit + "byte", fakeOrigin(), "test"));
        EXPECT_EQ(result, SimpleConfig::parseBytes("1" + unit + "bytes", fakeOrigin(), "test"));
    }

    // bad units
    try {
        SimpleConfig::parseBytes("100 dollars", fakeOrigin(), "test");
        FAIL() << "expected: ConfigExceptionBadValue";
    }
    catch (ConfigExceptionBadValue& e) {
        EXPECT_TRUE(boost::contains(e.what(), "size-in-bytes unit")) << "Exception message:" << e.what();
    }

    // bad number
    try {
        SimpleConfig::parseBytes("1 00 bytes", fakeOrigin(), "test");
        FAIL() << "expected: ConfigExceptionBadValue";
    }
    catch (ConfigExceptionBadValue& e) {
        EXPECT_TRUE(boost::contains(e.what(), "size-in-bytes number")) << "Exception message:" << e.what();
    }
}
