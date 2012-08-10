/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "test_fixture.h"
#include "configcpp/detail/simple_config_object.h"
#include "configcpp/detail/simple_config_origin.h"
#include "configcpp/detail/simple_config_list.h"
#include "configcpp/detail/string_reader.h"
#include "configcpp/config_parse_options.h"
#include "configcpp/config_resolve_options.h"
#include "configcpp/config_syntax.h"
#include "configcpp/config_origin.h"
#include "configcpp/config_includer.h"
#include "configcpp/config_includer_file.h"
#include "configcpp/config_value.h"
#include "configcpp/config_exception.h"
#include "configcpp/config.h"
#include "configcpp/config_util.h"

using namespace config;

class PublicApiTest : public TestFixture {
protected:
    static std::string defaultValueDesc() {
        return "hardcoded value";
    }

    void testFromValue(const AbstractConfigValuePtr& expectedValue, const ConfigVariant& createFrom) {
        checkEquals(expectedValue, std::dynamic_pointer_cast<AbstractConfigValue>(ConfigValue::fromAnyRef(createFrom)));
        EXPECT_EQ(defaultValueDesc(), ConfigValue::fromAnyRef(createFrom)->origin()->description());
        checkEquals(expectedValue, std::dynamic_pointer_cast<AbstractConfigValue>(ConfigValue::fromAnyRef(createFrom, "foo")));
        EXPECT_EQ("foo", ConfigValue::fromAnyRef(createFrom, "foo")->origin()->description());
    }

    void testFromPathMap(const AbstractConfigObjectPtr& expectedValue, const MapVariant& createFrom) {
        checkEquals(std::dynamic_pointer_cast<AbstractConfigValue>(expectedValue), std::dynamic_pointer_cast<AbstractConfigValue>(Config::parseMap(createFrom)->root()));
        EXPECT_EQ(defaultValueDesc(), Config::parseMap(createFrom)->origin()->description());
        checkEquals(std::dynamic_pointer_cast<AbstractConfigValue>(expectedValue), std::dynamic_pointer_cast<AbstractConfigValue>(Config::parseMap(createFrom, "foo")->root()));
        EXPECT_EQ("foo", Config::parseMap(createFrom, "foo")->origin()->description());
    }
};

TEST_F(PublicApiTest, basicLoadAndGet) {
    auto conf = Config::load(resourcePath() + "/test01");

    conf->getInt("ints.fortyTwo");
    auto child = conf->getConfig("ints");
    child->getInt("fortyTwo");
    conf->getMilliseconds("durations.halfSecond");

    // should have used system variables
    auto home = getenv("HOME");
    if (home) {
        EXPECT_EQ(home, conf->getString("system.home"));
    }
}

TEST_F(PublicApiTest, noSystemVariables) {
    // should not have used system variables
    auto conf = Config::parseFileAnySyntax(resourcePath() + "/test01")->resolve(ConfigResolveOptions::noSystem());

    EXPECT_THROW(conf->getString("system.home"), ConfigExceptionMissing);
}

TEST_F(PublicApiTest, canLimitLoadToJson) {
    auto options = ConfigParseOptions::defaults()->setSyntax(ConfigSyntax::JSON);
    auto conf = Config::load(resourcePath() + "/test01", options, ConfigResolveOptions::defaults());

    EXPECT_EQ(1, conf->getInt("fromJson1"));
    EXPECT_THROW(conf->getInt("ints.fortyTwo"), ConfigExceptionMissing);
}

TEST_F(PublicApiTest, canLimitLoadToConf) {
    auto options = ConfigParseOptions::defaults()->setSyntax(ConfigSyntax::CONF);
    auto conf = Config::load(resourcePath() + "/test01", options, ConfigResolveOptions::defaults());

    EXPECT_EQ(42, conf->getInt("ints.fortyTwo"));
    EXPECT_THROW(conf->getInt("fromJson1"), ConfigExceptionMissing);
}

TEST_F(PublicApiTest, emptyConfigs) {
    EXPECT_TRUE(Config::emptyConfig()->empty());
    EXPECT_EQ("empty config", Config::emptyConfig()->origin()->description());
    EXPECT_TRUE(Config::emptyConfig("foo")->empty());
    EXPECT_EQ("foo", Config::emptyConfig("foo")->origin()->description());
}

TEST_F(PublicApiTest, fromBoolean) {
    testFromValue(boolValue(true), true);
    testFromValue(boolValue(false), false);
}

TEST_F(PublicApiTest, fromNull) {
    testFromValue(nullValue(), null());
}

TEST_F(PublicApiTest, fromNumbers) {
    testFromValue(intValue(5), 5);
    testFromValue(int64Value(6), 6LL);
    testFromValue(doubleValue(3.14), 3.14);
}

TEST_F(PublicApiTest, fromString) {
    testFromValue(stringValue("hello world"), std::string("hello world"));
}

TEST_F(PublicApiTest, fromMap) {
    MapAbstractConfigValue aMapValue({{"a", intValue(1)}, {"b", intValue(2)}, {"c", intValue(3)}});
    MapVariant createFrom({{"a", 1}, {"b", 2}, {"c", 3}});
    testFromValue(SimpleConfigObject::make_instance(fakeOrigin(), MapAbstractConfigValue()), MapVariant());
    testFromValue(SimpleConfigObject::make_instance(fakeOrigin(), aMapValue), createFrom);

    EXPECT_EQ("hardcoded value", ConfigValue::fromMap(createFrom)->origin()->description());
    EXPECT_EQ("foo", ConfigValue::fromMap(createFrom, "foo")->origin()->description());
}

TEST_F(PublicApiTest, fromCollection) {
    VectorAbstractConfigValue aVectorValue({intValue(1), intValue(2), intValue(3)});
    VectorVariant createFrom({1, 2, 3});
    testFromValue(SimpleConfigList::make_instance(fakeOrigin(), VectorAbstractConfigValue()), VectorVariant());
    testFromValue(SimpleConfigList::make_instance(fakeOrigin(), aVectorValue), createFrom);

    EXPECT_EQ("hardcoded value", ConfigValue::fromVector(createFrom)->origin()->description());
    EXPECT_EQ("foo", ConfigValue::fromVector(createFrom, "foo")->origin()->description());
}

TEST_F(PublicApiTest, roundTripUnwrap) {
    auto conf = Config::load(resourcePath() + "/test01");
    EXPECT_TRUE(conf->root()->size() > 4); // "has a lot of stuff in it"
    auto unwrapped = conf->root()->unwrapped();
    auto rewrapped = ConfigValue::fromMap(variant_get<MapVariant>(unwrapped), conf->origin()->description());
    auto reunwrapped = rewrapped->unwrapped();
    checkEquals(std::dynamic_pointer_cast<ConfigBase>(conf->root()), std::dynamic_pointer_cast<ConfigBase>(rewrapped));
    EXPECT_TRUE(boost::apply_visitor(VariantEquals(reunwrapped), unwrapped));
}

TEST_F(PublicApiTest, fromPathMap) {
    // first the same tests as with fromMap, but use parseMap
    MapAbstractConfigValue aMapValue({{"a", intValue(1)}, {"b", intValue(2)}, {"c", intValue(3)}});
    MapVariant createFrom({{"a", 1}, {"b", 2}, {"c", 3}});
    testFromPathMap(SimpleConfigObject::make_instance(fakeOrigin(), MapAbstractConfigValue()), MapVariant());
    testFromPathMap(SimpleConfigObject::make_instance(fakeOrigin(), aMapValue), createFrom);

    EXPECT_EQ("hardcoded value", Config::parseMap(createFrom)->origin()->description());
    EXPECT_EQ("foo", Config::parseMap(createFrom, "foo")->origin()->description());

    // now some tests with paths; be sure to test nested path maps
    MapVariant simplePathMapValue({{"x.y", 4}, {"z", 5}});
    MapVariant pathMapValue({{"a.c", 1}, {"b", simplePathMapValue}});

    auto conf = Config::parseMap(pathMapValue);

    EXPECT_EQ(2, conf->root()->size());
    EXPECT_EQ(4, conf->getInt("b.x.y"));
    EXPECT_EQ(5, conf->getInt("b.z"));
    EXPECT_EQ(1, conf->getInt("a.c"));
}

TEST_F(PublicApiTest, brokenPathMap) {
    // "a" is both number 1 and an object
    MapVariant pathMapValue({{"a", 1}, {"a.b", 2}});
    EXPECT_THROW(Config::parseMap(pathMapValue), ConfigExceptionBugOrBroken);
}

TEST_F(PublicApiTest, defaultParseOptions) {
    auto d = ConfigParseOptions::defaults();
    EXPECT_TRUE(d->getAllowMissing());
    EXPECT_FALSE(d->getIncluder());
    EXPECT_EQ("", d->getOriginDescription());
    EXPECT_TRUE(d->getSyntax() == ConfigSyntax::NONE);
}

TEST_F(PublicApiTest, allowMissing) {
    try {
        Config::parseFile(resourcePath() + "/nonexistent.conf", ConfigParseOptions::defaults()->setAllowMissing(false));
        FAIL() << "expected: ConfigExceptionIO";
    }
    catch (ConfigExceptionIO& e) {
        EXPECT_TRUE(boost::contains(e.what(), "No such") ||
                    boost::contains(e.what(), "not found") ||
                    boost::contains(e.what(), "were found")) << "Exception message:" << e.what();
    }

    auto conf = Config::parseFile(resourcePath() + "/nonexistent.conf", ConfigParseOptions::defaults()->setAllowMissing(true));
    EXPECT_TRUE(conf->empty());
}

TEST_F(PublicApiTest, allowMissingFileAnySyntax) {
    try {
        Config::parseFileAnySyntax(resourcePath() + "/nonexistent", ConfigParseOptions::defaults()->setAllowMissing(false));
        FAIL() << "expected: ConfigExceptionIO";
    }
    catch (ConfigExceptionIO& e) {
        EXPECT_TRUE(boost::contains(e.what(), "No such") ||
                    boost::contains(e.what(), "not found") ||
                    boost::contains(e.what(), "were found")) << "Exception message:" << e.what();
    }

    auto conf = Config::parseFileAnySyntax(resourcePath() + "/nonexistent", ConfigParseOptions::defaults()->setAllowMissing(true));
    EXPECT_TRUE(conf->empty());
}

TEST_F(PublicApiTest, includesCanBeMissingThoughFileCannot) {
    // test03.conf contains some nonexistent includes. check that
    // setAllowMissing on the file (which is not missing) doesn't
    // change that the includes are allowed to be missing.
    // This can break because some options might "propagate" through
    // to includes, but we don't want them all to do so.
    auto conf = Config::parseFile(resourcePath() + "/test03.conf", ConfigParseOptions::defaults()->setAllowMissing(false));
    EXPECT_EQ(42, conf->getInt("test01.booleans"));

    auto conf2 = Config::parseFile(resourcePath() + "/test03.conf", ConfigParseOptions::defaults()->setAllowMissing(true));
    checkEquals(std::dynamic_pointer_cast<ConfigBase>(conf), std::dynamic_pointer_cast<ConfigBase>(conf2));
}

namespace {

enum class IncludeKind : uint32_t {
    IncludeKindHeuristic,
    IncludeKindFile
};

struct Included {
    Included(const std::string& name, const ConfigIncluderPtr& fallback, IncludeKind kind) :
        name(name),
        fallback(fallback),
        kind(kind) {
    }
    std::string name;
    ConfigIncluderPtr fallback;
    IncludeKind kind;
    bool operator== (const Included& other) const {
        return name == other.name;
    }
};
typedef std::vector<Included> IncludedList;

class RecordingIncluder : public virtual ConfigIncluder, public ConfigBase {
public:
    CONFIG_CLASS(RecordingIncluder);

    RecordingIncluder(const ConfigIncluderPtr& fallback, IncludedList& included) :
        fallback(fallback),
        included(included) {
    }

    virtual ConfigObjectPtr include(const ConfigIncludeContextPtr& context, const std::string& name) override {
        included.push_back(Included(name, fallback, IncludeKind::IncludeKindHeuristic));
        return fallback->include(context, name);
    }

    virtual ConfigIncluderPtr withFallback(const ConfigIncluderPtr& fallback) override {
        if (this->fallback == fallback) {
            return shared_from_this();
        }
        else if (!this->fallback) {
            return RecordingIncluder::make_instance(fallback, included);
        }
        else {
            return RecordingIncluder::make_instance(this->fallback->withFallback(fallback), included);
        }
    }

protected:
    ConfigIncluderPtr fallback;
    IncludedList& included;
};

class RecordingFullIncluder : public virtual ConfigIncluderFile, public RecordingIncluder {
public:
    CONFIG_CLASS(RecordingFullIncluder);

    RecordingFullIncluder(const ConfigIncluderPtr& fallback, IncludedList& included) :
        RecordingIncluder(fallback, included) {
    }

    virtual ConfigObjectPtr includeFile(const ConfigIncludeContextPtr& context, const std::string& file) override {
        included.push_back(Included("file(" + file + ")", fallback, IncludeKind::IncludeKindFile));
        return std::dynamic_pointer_cast<ConfigIncluderFile>(fallback)->includeFile(context, file);
    }
};

}

TEST_F(PublicApiTest, includersAreUsedWithFiles) {
    IncludedList included;
    auto includer = RecordingIncluder::make_instance(nullptr, included);

    Config::parseFile(resourcePath() + "/test03.conf", ConfigParseOptions::defaults()->setIncluder(includer)->setAllowMissing(false));

    VectorString includedNames;
    for (auto& i : included) {
        includedNames.push_back(i.name);
    }

    EXPECT_TRUE(VectorString({
        "test01", "test02.conf", "equiv01/original.json",
        "nothere", "nothere.conf", "nothere.json", "nothere.properties",
        "test03-included.conf", "test03-included.conf"}) == includedNames);
}

TEST_F(PublicApiTest, includersAreUsedRecursivelyWithFiles) {
    // includes.conf has recursive includes in it
    IncludedList included;
    auto includer = RecordingIncluder::make_instance(nullptr, included);

    Config::parseFile(resourcePath() + "/equiv03/includes.conf", ConfigParseOptions::defaults()->setIncluder(includer)->setAllowMissing(false));

    VectorString includedNames;
    for (auto& i : included) {
        includedNames.push_back(i.name);
    }

    EXPECT_TRUE(VectorString({
        "letters/a.conf", "numbers/1.conf", "numbers/2",
        "letters/b.json", "letters/c", "root/foo.conf"}) == includedNames);
}

TEST_F(PublicApiTest, fullIncluderNotUsedWithoutNewSyntax) {
    IncludedList included;
    auto includer = RecordingIncluder::make_instance(nullptr, included);

    Config::parseFile(resourcePath() + "/equiv03/includes.conf", ConfigParseOptions::defaults()->setIncluder(includer)->setAllowMissing(false));

    VectorString includedNames;
    for (auto& i : included) {
        includedNames.push_back(i.name);
    }

    IncludedList includedFull;
    auto fullIncluder = RecordingFullIncluder::make_instance(nullptr, includedFull);

    Config::parseFile(resourcePath() + "/equiv03/includes.conf", ConfigParseOptions::defaults()->setIncluder(fullIncluder)->setAllowMissing(false));

    EXPECT_TRUE(included == includedFull);
}

TEST_F(PublicApiTest, stringParsing) {
    auto conf = Config::parseString("{ a : b }", ConfigParseOptions::defaults());
    EXPECT_EQ("b", conf->getString("a"));
}

TEST_F(PublicApiTest, readerParsing) {
    auto conf = Config::parseReader(StringReader::make_instance("{ a : b }"), ConfigParseOptions::defaults());
    EXPECT_EQ("b", conf->getString("a"));
}

TEST_F(PublicApiTest, anySyntax) {
    // test01 has all three syntaxes; first load with basename
    auto conf = Config::parseFileAnySyntax(resourcePath() + "/test01", ConfigParseOptions::defaults());
    EXPECT_EQ(42, conf->getInt("ints.fortyTwo"));
    EXPECT_EQ("A", conf->getString("fromJsonA"));

    // now include a suffix, should only load one of them
    auto onlyJson = Config::parseFileAnySyntax(resourcePath() + "/test01.json", ConfigParseOptions::defaults());
    EXPECT_FALSE(onlyJson->hasPath("ints.fortyTwo"));
    EXPECT_EQ(1, onlyJson->getInt("fromJson1"));
    EXPECT_EQ("A", onlyJson->getString("fromJsonA"));
}

TEST_F(PublicApiTest, splitAndJoinPath) {
    // the actual join-path logic should be tested OK in the non-public-API tests,
    // this is just to test the public wrappers.
    EXPECT_EQ("\"\".a.b.\"$\"", ConfigUtil::joinPath(VectorString({"", "a", "b", "$"})));
    EXPECT_TRUE(VectorString({"", "a", "b", "$"}) == ConfigUtil::splitPath("\"\".a.b.\"$\""));

    // invalid stuff throws
    EXPECT_THROW(ConfigUtil::splitPath("$"), ConfigException);
    EXPECT_THROW(ConfigUtil::joinPath(VectorString()), ConfigException);
}

TEST_F(PublicApiTest, quoteString) {
    // the actual quote logic should be tested OK in the non-public-API tests,
    // this is just to test the public wrapper.

    EXPECT_EQ("\"\"", ConfigUtil::quoteString(""));
    EXPECT_EQ("\"a\"", ConfigUtil::quoteString("a"));
    EXPECT_EQ("\"\\n\"", ConfigUtil::quoteString("\n"));
}

TEST_F(PublicApiTest, detectIncludeCycle) {
    try {
        Config::load(resourcePath() + "/cycle");
        FAIL() << "expected: ConfigExceptionParse";
    }
    catch (ConfigExceptionParse& e) {
        EXPECT_TRUE(boost::contains(e.what(), "include statements nested")) << "Exception message:" << e.what();
    }
}
