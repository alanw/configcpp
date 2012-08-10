/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "test_fixture.h"
#include "configcpp/detail/resolve_context.h"
#include "configcpp/detail/config_reference.h"
#include "configcpp/detail/config_delayed_merge.h"
#include "configcpp/detail/config_delayed_merge_object.h"
#include "configcpp/detail/abstract_config_object.h"
#include "configcpp/detail/simple_config_object.h"
#include "configcpp/detail/simple_config.h"
#include "configcpp/config.h"
#include "configcpp/config_syntax.h"
#include "configcpp/config_resolve_options.h"
#include "configcpp/config_render_options.h"
#include "configcpp/config_list.h"
#include "configcpp/config_origin.h"
#include "configcpp/config_parse_options.h"

using namespace config;

class ConfigTest : public TestFixture {
protected:
    AbstractConfigValuePtr resolveNoSystem(const AbstractConfigValuePtr& v, const AbstractConfigObjectPtr& root) {
        return ResolveContext::resolve(v, root, ConfigResolveOptions::noSystem());
    }

    ConfigPtr resolveNoSystem(const SimpleConfigPtr& v, const SimpleConfigPtr& root) {
        return std::static_pointer_cast<AbstractConfigObject>(ResolveContext::resolve(std::dynamic_pointer_cast<AbstractConfigValue>(v->root()), std::dynamic_pointer_cast<AbstractConfigObject>(root->root()), ConfigResolveOptions::noSystem()))->toConfig();
    }

    AbstractConfigObjectPtr mergeUnresolved(const VectorAbstractConfigObject& toMerge) {
        if (toMerge.empty()) {
            return SimpleConfigObject::makeEmpty();
        }
        AbstractConfigObjectPtr merge;
        for (auto m = toMerge.begin(); m != toMerge.end(); ++m) {
            if (m == toMerge.begin()) {
                merge = *m;
            }
            else {
                merge = std::dynamic_pointer_cast<AbstractConfigObject>(std::static_pointer_cast<AbstractConfigObject>(merge)->withFallback(*m));
            }
        }
        return merge;
    }

    AbstractConfigObjectPtr merge(const VectorAbstractConfigObject& toMerge) {
        auto obj = mergeUnresolved(toMerge);
        return std::dynamic_pointer_cast<AbstractConfigObject>(resolveNoSystem(obj, obj));
    }

    AbstractConfigObjectPtr cycleObject() {
        return parseObject("{\n"
                           " \"foo\" : ${bar},\n"
                           " \"bar\" : ${a.b.c},\n"
                           " \"a\" : { \"b\" : { \"c\" : ${foo} } }\n"
                           "}\n"
                           );
    }

    bool ignoresFallbacks(const ConfigMergeablePtr& m) {
        if (instanceof<AbstractConfigValue>(m)) {
            return std::dynamic_pointer_cast<AbstractConfigValue>(m)->ignoresFallbacks();
        }
        else if (instanceof<SimpleConfig>(m)) {
            return std::dynamic_pointer_cast<AbstractConfigValue>(std::dynamic_pointer_cast<SimpleConfig>(m)->root())->ignoresFallbacks();
        }
        else {
            ADD_FAILURE() << "Unexpected value";
            return false;
        }
    }

    void testIgnoredMergesDoNothing(const ConfigMergeablePtr& nonEmpty) {
        // falling back to a primitive once should switch us to "ignoreFallbacks" mode
        // and then twice should "return this". Falling back to an empty object should
        // return this unless the empty object was ignoreFallbacks and then we should
        // "catch" its ignoreFallbacks.

        // some of what this tests is just optimization, not API contract (withFallback
        // can return a new object anytime it likes) but want to be sure we do the
        // optimizations.

        auto empty = SimpleConfigObject::makeEmpty(nullptr);
        auto primitive = intValue(42);
        auto emptyIgnoringFallbacks = empty->withFallback(primitive);
        auto nonEmptyIgnoringFallbacks = nonEmpty->withFallback(primitive);

        EXPECT_FALSE(empty->ignoresFallbacks());
        EXPECT_TRUE(primitive->ignoresFallbacks());
        EXPECT_TRUE(std::dynamic_pointer_cast<AbstractConfigValue>(emptyIgnoringFallbacks)->ignoresFallbacks());
        EXPECT_FALSE(ignoresFallbacks(nonEmpty));
        EXPECT_TRUE(ignoresFallbacks(nonEmptyIgnoringFallbacks));

        checkNotSame(std::dynamic_pointer_cast<ConfigBase>(nonEmpty), std::dynamic_pointer_cast<ConfigBase>(nonEmptyIgnoringFallbacks));
        checkNotSame(std::dynamic_pointer_cast<ConfigBase>(empty), std::dynamic_pointer_cast<ConfigBase>(emptyIgnoringFallbacks));

        // falling back from one object to another should not make us ignore fallbacks
        EXPECT_FALSE(ignoresFallbacks(nonEmpty->withFallback(empty)));
        EXPECT_FALSE(ignoresFallbacks(empty->withFallback(nonEmpty)));
        EXPECT_FALSE(ignoresFallbacks(empty->withFallback(empty)));
        EXPECT_FALSE(ignoresFallbacks(nonEmpty->withFallback(nonEmpty)));

        // falling back from primitive just returns this
        checkSame(std::dynamic_pointer_cast<ConfigBase>(primitive), std::dynamic_pointer_cast<ConfigBase>(primitive->withFallback(empty)));
        checkSame(std::dynamic_pointer_cast<ConfigBase>(primitive), std::dynamic_pointer_cast<ConfigBase>(primitive->withFallback(nonEmpty)));
        checkSame(std::dynamic_pointer_cast<ConfigBase>(primitive), std::dynamic_pointer_cast<ConfigBase>(primitive->withFallback(nonEmptyIgnoringFallbacks)));

        // falling back again from an ignoreFallbacks should be a no-op, return this
        checkSame(std::dynamic_pointer_cast<ConfigBase>(nonEmptyIgnoringFallbacks), std::dynamic_pointer_cast<ConfigBase>(nonEmptyIgnoringFallbacks->withFallback(empty)));
        checkSame(std::dynamic_pointer_cast<ConfigBase>(nonEmptyIgnoringFallbacks), std::dynamic_pointer_cast<ConfigBase>(nonEmptyIgnoringFallbacks->withFallback(primitive)));
        checkSame(std::dynamic_pointer_cast<ConfigBase>(emptyIgnoringFallbacks), std::dynamic_pointer_cast<ConfigBase>(emptyIgnoringFallbacks->withFallback(empty)));
        checkSame(std::dynamic_pointer_cast<ConfigBase>(emptyIgnoringFallbacks), std::dynamic_pointer_cast<ConfigBase>(emptyIgnoringFallbacks->withFallback(primitive)));
    }
};

TEST_F(ConfigTest, mergeTrivial) {
    auto obj1 = parseObject("{ \"a\" : 1 }");
    auto obj2 = parseObject("{ \"b\" : 2 }");
    auto merged = merge({obj1, obj2})->toConfig();

    EXPECT_EQ(1, merged->getInt("a"));
    EXPECT_EQ(2, merged->getInt("b"));
    EXPECT_EQ(2, merged->root()->size());
}

TEST_F(ConfigTest, mergeEmpty) {
    auto merged = merge({})->toConfig();

    EXPECT_EQ(0, merged->root()->size());
}

TEST_F(ConfigTest, mergeOne) {
    auto obj1 = parseObject("{ \"a\" : 1 }");
    auto merged = merge({obj1})->toConfig();

    EXPECT_EQ(1, merged->getInt("a"));
    EXPECT_EQ(1, merged->root()->size());
}

TEST_F(ConfigTest, mergeOverride) {
    auto obj1 = parseObject("{ \"a\" : 1 }");
    auto obj2 = parseObject("{ \"a\" : 2 }");
    auto merged = merge({obj1, obj2})->toConfig();

    EXPECT_EQ(1, merged->getInt("a"));
    EXPECT_EQ(1, merged->root()->size());

    auto merged2 = merge({obj2, obj1})->toConfig();

    EXPECT_EQ(2, merged2->getInt("a"));
    EXPECT_EQ(1, merged2->root()->size());
}

TEST_F(ConfigTest, mergeN) {
    auto obj1 = parseObject("{ \"a\" : 1 }");
    auto obj2 = parseObject("{ \"b\" : 2 }");
    auto obj3 = parseObject("{ \"c\" : 3 }");
    auto obj4 = parseObject("{ \"d\" : 4 }");

    auto merged = merge({obj1, obj2, obj3, obj4})->toConfig();

    EXPECT_EQ(1, merged->getInt("a"));
    EXPECT_EQ(2, merged->getInt("b"));
    EXPECT_EQ(3, merged->getInt("c"));
    EXPECT_EQ(4, merged->getInt("d"));
    EXPECT_EQ(4, merged->root()->size());
}

TEST_F(ConfigTest, mergeOverrideN) {
    auto obj1 = parseObject("{ \"a\" : 1 }");
    auto obj2 = parseObject("{ \"a\" : 2 }");
    auto obj3 = parseObject("{ \"a\" : 3 }");
    auto obj4 = parseObject("{ \"a\" : 4 }");

    auto merged = merge({obj1, obj2, obj3, obj4})->toConfig();

    EXPECT_EQ(1, merged->getInt("a"));
    EXPECT_EQ(1, merged->root()->size());

    auto merged2 = merge({obj4, obj3, obj2, obj1})->toConfig();

    EXPECT_EQ(4, merged2->getInt("a"));
    EXPECT_EQ(1, merged2->root()->size());
}

TEST_F(ConfigTest, mergeNested) {
    auto obj1 = parseObject("{ \"root\" : { \"a\" : 1, \"z\" : 101 } }");
    auto obj2 = parseObject("{ \"root\" : { \"b\" : 2, \"z\" : 102 } }");
    auto merged = merge({obj1, obj2})->toConfig();

    EXPECT_EQ(1, merged->getInt("root.a"));
    EXPECT_EQ(2, merged->getInt("root.b"));
    EXPECT_EQ(101, merged->getInt("root.z"));
    EXPECT_EQ(1, merged->root()->size());
    EXPECT_EQ(3, merged->getConfig("root")->root()->size());
}

TEST_F(ConfigTest, mergeWithEmpty) {
    auto obj1 = parseObject("{ \"a\" : 1 }");
    auto obj2 = parseObject("{ }");
    auto merged = merge({obj1, obj2})->toConfig();

    EXPECT_EQ(1, merged->getInt("a"));
    EXPECT_EQ(1, merged->root()->size());

    auto merged2 = merge({obj2, obj1})->toConfig();

    EXPECT_EQ(1, merged2->getInt("a"));
    EXPECT_EQ(1, merged2->root()->size());
}

TEST_F(ConfigTest, mergeOverrideObjectAndPrimitive) {
    auto obj1 = parseObject("{ \"a\" : 1 }");
    auto obj2 = parseObject("{ \"a\" : { \"b\" : 42 } }");
    auto merged = merge({obj1, obj2})->toConfig();

    EXPECT_EQ(1, merged->getInt("a"));
    EXPECT_EQ(1, merged->root()->size());

    auto merged2 = merge({obj2, obj1})->toConfig();

    EXPECT_EQ(42, merged2->getConfig("a")->getInt("b"));
    EXPECT_EQ(42, merged2->getInt("a.b"));
    EXPECT_EQ(1, merged2->root()->size());
    EXPECT_EQ(1, merged2->getObject("a")->size());
}

TEST_F(ConfigTest, mergeOverrideObjectAndSubstitution) {
    auto obj1 = parseObject("{ \"a\" : 1 }");
    auto obj2 = parseObject("{ \"a\" : { \"b\" : ${c} }, \"c\" : 42 }");
    auto merged = merge({obj1, obj2})->toConfig();

    EXPECT_EQ(1, merged->getInt("a"));
    EXPECT_EQ(2, merged->root()->size());

    auto merged2 = merge({obj2, obj1})->toConfig();

    EXPECT_EQ(42, merged2->getConfig("a")->getInt("b"));
    EXPECT_EQ(42, merged2->getInt("a.b"));
    EXPECT_EQ(2, merged2->root()->size());
    EXPECT_EQ(1, merged2->getObject("a")->size());
}

TEST_F(ConfigTest, mergeObjectThenPrimitiveThenObject) {
    // the semantic here is that the primitive blocks the
    // object that occurs at lower priority. This is consistent
    // with duplicate keys in the same file.
    auto obj1 = parseObject("{ \"a\" : { \"b\" : 42 } }");
    auto obj2 = parseObject("{ \"a\" : 2 }");
    auto obj3 = parseObject("{ \"a\" : { \"b\" : 43, \"c\" : 44 } }");

    auto merged = merge({obj1, obj2, obj3})->toConfig();

    EXPECT_EQ(42, merged->getInt("a.b"));
    EXPECT_EQ(1, merged->root()->size());
    EXPECT_EQ(1, merged->getObject("a")->size());

    auto merged2 = merge({obj3, obj2, obj1})->toConfig();

    EXPECT_EQ(43, merged2->getInt("a.b"));
    EXPECT_EQ(44, merged2->getInt("a.c"));
    EXPECT_EQ(1, merged2->root()->size());
    EXPECT_EQ(2, merged2->getObject("a")->size());
}

TEST_F(ConfigTest, mergeObjectThenSubstitutionThenObject) {
    // the semantic here is that the primitive blocks the
    // object that occurs at lower priority. This is consistent
    // with duplicate keys in the same file.
    auto obj1 = parseObject("{ \"a\" : { \"b\" : ${f} } }");
    auto obj2 = parseObject("{ \"a\" : 2 }");
    auto obj3 = parseObject("{ \"a\" : { \"b\" : ${d}, \"c\" : ${e} }, \"d\" : 43, \"e\" : 44, \"f\" : 42 }");

    auto merged = merge({obj1, obj2, obj3})->toConfig();

    EXPECT_EQ(42, merged->getInt("a.b"));
    EXPECT_EQ(4, merged->root()->size());
    EXPECT_EQ(1, merged->getObject("a")->size());

    auto merged2 = merge({obj3, obj2, obj1})->toConfig();

    EXPECT_EQ(43, merged2->getInt("a.b"));
    EXPECT_EQ(44, merged2->getInt("a.c"));
    EXPECT_EQ(4, merged2->root()->size());
    EXPECT_EQ(2, merged2->getObject("a")->size());
}

TEST_F(ConfigTest, mergePrimitiveThenObjectThenPrimitive) {
    // the primitive should override the object
    auto obj1 = parseObject("{ \"a\" : 1 }");
    auto obj2 = parseObject("{ \"a\" : { \"b\" : 42 } }");
    auto obj3 = parseObject("{ \"a\" : 3 }");

    auto merged = merge({obj1, obj2, obj3})->toConfig();

    EXPECT_EQ(1, merged->getInt("a"));
    EXPECT_EQ(1, merged->root()->size());
}

TEST_F(ConfigTest, mergeSubstitutionThenObjectThenSubstitution) {
    // the primitive should override the object
    auto obj1 = parseObject("{ \"a\" : ${b}, \"b\" : 1 }");
    auto obj2 = parseObject("{ \"a\" : { \"b\" : 42 } }");
    auto obj3 = parseObject("{ \"a\" : ${c}, \"c\" : 2 }");

    auto merged = merge({obj1, obj2, obj3})->toConfig();

    EXPECT_EQ(1, merged->getInt("a"));
    EXPECT_EQ(3, merged->root()->size());
}

TEST_F(ConfigTest, mergeSubstitutedValues) {
    // the primitive should override the object
    auto obj1 = parseObject("{ \"a\" : { \"x\" : 1, \"z\" : 4 }, \"c\" : ${a} }");
    auto obj2 = parseObject("{ \"b\" : { \"y\" : 2, \"z\" : 5 }, \"c\" : ${b} }");

    auto resolved = merge({obj1, obj2})->toConfig();

    EXPECT_EQ(3, resolved->getObject("c")->size());
    EXPECT_EQ(1, resolved->getInt("c.x"));
    EXPECT_EQ(2, resolved->getInt("c.y"));
    EXPECT_EQ(4, resolved->getInt("c.z"));
}

TEST_F(ConfigTest, mergeObjectWithSubstituted) {
    // the primitive should override the object
    auto obj1 = parseObject("{ \"a\" : { \"x\" : 1, \"z\" : 4 }, \"c\" : { \"z\" : 42 } }");
    auto obj2 = parseObject("{ \"b\" : { \"y\" : 2, \"z\" : 5 }, \"c\" : ${b} }");

    auto resolved = merge({obj1, obj2})->toConfig();

    EXPECT_EQ(2, resolved->getObject("c")->size());
    EXPECT_EQ(2, resolved->getInt("c.y"));
    EXPECT_EQ(42, resolved->getInt("c.z"));

    auto resolved2 = merge({obj2, obj1})->toConfig();

    EXPECT_EQ(2, resolved2->getObject("c")->size());
    EXPECT_EQ(2, resolved2->getInt("c.y"));
    EXPECT_EQ(5, resolved2->getInt("c.z"));
}

TEST_F(ConfigTest, mergeHidesCycles) {
    // the point here is that we should not try to evaluate a substitution
    // that's been overridden, and thus not end up with a cycle as long
    // as we override the problematic link in the cycle.
    try {
        resolveNoSystem(subst("foo"), cycleObject());
        FAIL() << "expected: ConfigExceptionUnresolvedSubstitution";
    }
    catch (ConfigExceptionUnresolvedSubstitution& e) {
        EXPECT_TRUE(boost::contains(e.what(), "cycle"));
    }

    auto fixUpCycle = parseObject(" { \"a\" : { \"b\" : { \"c\" : 57 } } } ");
    auto merged = mergeUnresolved({fixUpCycle, cycleObject()});
    auto v = resolveNoSystem(subst("foo"), merged);
    checkEquals(intValue(57), v);
}

TEST_F(ConfigTest, mergeWithObjectInFrontKeepsCycles) {
    // the point here is that if our eventual value will be an object, then
    // we have to evaluate the substitution to see if it's an object to merge,
    // so we don't avoid the cycle.
    try {
        resolveNoSystem(subst("foo"), cycleObject());
        FAIL() << "expected: ConfigExceptionUnresolvedSubstitution";
    }
    catch (ConfigExceptionUnresolvedSubstitution& e) {
        EXPECT_TRUE(boost::contains(e.what(), "cycle"));
    }

    auto fixUpCycle = parseObject(" { \"a\" : { \"b\" : { \"c\" : { \"q\" : \"u\" } } } } ");
    auto merged = mergeUnresolved({fixUpCycle, cycleObject()});
    EXPECT_THROW(resolveNoSystem(subst("foo"), merged), ConfigExceptionUnresolvedSubstitution);
}

TEST_F(ConfigTest, mergeSeriesOfSubstitutions) {
    auto obj1 = parseObject("{ \"a\" : { \"x\" : 1, \"q\" : 4 }, \"j\" : ${a} }");
    auto obj2 = parseObject("{ \"b\" : { \"y\" : 2, \"q\" : 5 }, \"j\" : ${b} }");
    auto obj3 = parseObject("{ \"c\" : { \"z\" : 3, \"q\" : 6 }, \"j\" : ${c} }");

    auto resolved = merge({obj1, obj2, obj3})->toConfig();

    EXPECT_EQ(4, resolved->getObject("j")->size());
    EXPECT_EQ(1, resolved->getInt("j.x"));
    EXPECT_EQ(2, resolved->getInt("j.y"));
    EXPECT_EQ(3, resolved->getInt("j.z"));
    EXPECT_EQ(4, resolved->getInt("j.q"));
}

TEST_F(ConfigTest, mergePrimitiveAndTwoSubstitutions) {
    auto obj1 = parseObject("{ \"j\" : 42 }");
    auto obj2 = parseObject("{ \"b\" : { \"y\" : 2, \"q\" : 5 }, \"j\" : ${b} }");
    auto obj3 = parseObject("{ \"c\" : { \"z\" : 3, \"q\" : 6 }, \"j\" : ${c} }");

    auto resolved = merge({obj1, obj2, obj3})->toConfig();

    EXPECT_EQ(3, resolved->root()->size());
    EXPECT_EQ(42, resolved->getInt("j"));
    EXPECT_EQ(2, resolved->getInt("b.y"));
    EXPECT_EQ(3, resolved->getInt("c.z"));
}

TEST_F(ConfigTest, mergeObjectAndTwoSubstitutions) {
    auto obj1 = parseObject("{ \"j\" : { \"x\" : 1, \"q\" : 4 } }");
    auto obj2 = parseObject("{ \"b\" : { \"y\" : 2, \"q\" : 5 }, \"j\" : ${b} }");
    auto obj3 = parseObject("{ \"c\" : { \"z\" : 3, \"q\" : 6 }, \"j\" : ${c} }");

    auto resolved = merge({obj1, obj2, obj3})->toConfig();

    EXPECT_EQ(4, resolved->getObject("j")->size());
    EXPECT_EQ(1, resolved->getInt("j.x"));
    EXPECT_EQ(2, resolved->getInt("j.y"));
    EXPECT_EQ(3, resolved->getInt("j.z"));
    EXPECT_EQ(4, resolved->getInt("j.q"));
}

TEST_F(ConfigTest, mergeObjectSubstitutionObjectSubstitution) {
    auto obj1 = parseObject("{ \"j\" : { \"w\" : 1, \"q\" : 5 } }");
    auto obj2 = parseObject("{ \"b\" : { \"x\" : 2, \"q\" : 6 }, \"j\" : ${b} }");
    auto obj3 = parseObject("{ \"j\" : { \"y\" : 3, \"q\" : 7 } }");
    auto obj4 = parseObject("{ \"c\" : { \"z\" : 4, \"q\" : 8 }, \"j\" : ${c} }");

    auto resolved = merge({obj1, obj2, obj3, obj4})->toConfig();

    EXPECT_EQ(5, resolved->getObject("j")->size());
    EXPECT_EQ(1, resolved->getInt("j.w"));
    EXPECT_EQ(2, resolved->getInt("j.x"));
    EXPECT_EQ(3, resolved->getInt("j.y"));
    EXPECT_EQ(4, resolved->getInt("j.z"));
    EXPECT_EQ(5, resolved->getInt("j.q"));
}

TEST_F(ConfigTest, ignoredMergesDoNothing) {
    auto conf = parseConfig("{ a : 1 }");
    testIgnoredMergesDoNothing(conf);
}

TEST_F(ConfigTest, testNoMergeAcrossArray) {
    auto conf = parseConfig("a: {b:1}, a: [2,3], a:{c:4}");
    EXPECT_FALSE(conf->hasPath("a.b")) << "a.b found in: " << std::dynamic_pointer_cast<ConfigBase>(conf)->toString();
}

TEST_F(ConfigTest, testNoMergeAcrossUnresolvedArray) {
    auto conf = parseConfig("a: {b:1}, a: [2,${x}], a:{c:4}, x: 42");
    EXPECT_FALSE(conf->hasPath("a.b")) << "a.b found in: " << std::dynamic_pointer_cast<ConfigBase>(conf)->toString();
    EXPECT_TRUE(conf->hasPath("a.c")) << "a.c not found in: " << std::dynamic_pointer_cast<ConfigBase>(conf)->toString();
}

TEST_F(ConfigTest, integerRangeChecks) {
    auto conf = parseConfig("{ tooNegative: " +
                            boost::lexical_cast<std::string>(static_cast<int64_t>(std::numeric_limits<int32_t>::min() - 1LL)) +
                            ", tooPositive: " +
                            boost::lexical_cast<std::string>(static_cast<int64_t>(std::numeric_limits<int32_t>::max() + 1LL)) +
                            "}");

    try {
        conf->getInt("tooNegative");
        FAIL() << "expected: ConfigExceptionWrongType";
    }
    catch (ConfigExceptionWrongType& e) {
        EXPECT_TRUE(boost::contains(e.what(), "range"));
    }

    try {
        conf->getInt("tooPositive");
        FAIL() << "expected: ConfigExceptionWrongType";
    }
    catch (ConfigExceptionWrongType& e) {
        EXPECT_TRUE(boost::contains(e.what(), "range"));
    }
}

TEST_F(ConfigTest, test01Getting) {
    auto conf = Config::load(resourcePath() + "/test01");

    // get all the primitive types
    EXPECT_EQ(42, conf->getInt("ints.fortyTwo"));
    EXPECT_EQ(42, conf->getInt("ints.fortyTwoAgain"));
    EXPECT_EQ(42LL, conf->getInt64("ints.fortyTwoAgain"));
    EXPECT_DOUBLE_EQ(42.1, conf->getDouble("floats.fortyTwoPointOne"));
    EXPECT_DOUBLE_EQ(42.1, conf->getDouble("floats.fortyTwoPointOneAgain"));
    EXPECT_EQ("abcd", conf->getString("strings.abcd"));
    EXPECT_EQ("abcd", conf->getString("strings.abcdAgain"));
    EXPECT_EQ("null bar 42 baz true 3.14 hi", conf->getString("strings.concatenated"));
    EXPECT_TRUE(conf->getBoolean("booleans.trueAgain"));
    EXPECT_FALSE(conf->getBoolean("booleans.falseAgain"));

    // to get null we have to use the get() method from Map,
    // which takes a key and not a path
    checkEquals(nullValue(), std::dynamic_pointer_cast<AbstractConfigValue>(conf->getObject("nulls")->get("null")));
    EXPECT_FALSE(conf->root()->get("notinthefile"));

    // get stuff with getValue
    checkEquals(intValue(42), std::dynamic_pointer_cast<AbstractConfigValue>(conf->getValue("ints.fortyTwo")));
    checkEquals(stringValue("abcd"), std::dynamic_pointer_cast<AbstractConfigValue>(conf->getValue("strings.abcd")));

    // get stuff with getAny
    EXPECT_EQ(42, variant_get<int32_t>(conf->getVariant("ints.fortyTwo")));
    EXPECT_EQ("abcd", variant_get<std::string>(conf->getVariant("strings.abcd")));
    EXPECT_FALSE(variant_get<bool>(conf->getVariant("booleans.falseAgain")));

    // get empty array as any type of array
    EXPECT_TRUE(conf->getVariantList("arrays.empty").empty());
    EXPECT_TRUE(conf->getIntList("arrays.empty").empty());
    EXPECT_TRUE(conf->getInt64List("arrays.empty").empty());
    EXPECT_TRUE(conf->getStringList("arrays.empty").empty());
    EXPECT_TRUE(conf->getInt64List("arrays.empty").empty());
    EXPECT_TRUE(conf->getDoubleList("arrays.empty").empty());
    EXPECT_TRUE(conf->getObjectList("arrays.empty").empty());
    EXPECT_TRUE(conf->getBooleanList("arrays.empty").empty());
    EXPECT_TRUE(conf->getList("arrays.empty")->empty());

    // get typed arrays
    EXPECT_TRUE(VectorInt({1, 2, 3}) == conf->getIntList("arrays.ofInt"));
    EXPECT_TRUE(VectorInt64({1, 2, 3}) == conf->getInt64List("arrays.ofInt"));
    EXPECT_TRUE(VectorString({"a", "b", "c"}) == conf->getStringList("arrays.ofString"));
    EXPECT_TRUE(VectorDouble({3.14, 4.14, 5.14}) == conf->getDoubleList("arrays.ofDouble"));
    EXPECT_TRUE(VectorVariant({null(), null(), null()}) == conf->getVariantList("arrays.ofNull"));
    EXPECT_TRUE(VectorBool({true, false}) == conf->getBooleanList("arrays.ofBoolean"));
    VectorVariant listOfLists = conf->getVariantList("arrays.ofArray");
    EXPECT_EQ(3, listOfLists.size());
    for (auto& list : listOfLists) {
        VectorVariant subList = variant_get<VectorVariant>(list);
        ASSERT_EQ(3, subList.size());
        EXPECT_EQ("a", variant_get<std::string>(subList[0]));
        EXPECT_EQ("b", variant_get<std::string>(subList[1]));
        EXPECT_EQ("c", variant_get<std::string>(subList[2]));
    }
    EXPECT_EQ(3, conf->getObjectList("arrays.ofObject").size());

    EXPECT_TRUE(VectorString({"a", "b"}) == conf->getStringList("arrays.firstElementNotASubst"));

    // plain getList should work
    auto intList = conf->getList("arrays.ofInt");
    ASSERT_EQ(3, intList->size());
    checkEquals(intValue(1), std::dynamic_pointer_cast<ConfigBase>(intList->at(0)));
    checkEquals(intValue(2), std::dynamic_pointer_cast<ConfigBase>(intList->at(1)));
    checkEquals(intValue(3), std::dynamic_pointer_cast<ConfigBase>(intList->at(2)));

    auto stringList = conf->getList("arrays.ofString");
    ASSERT_EQ(3, stringList->size());
    checkEquals(stringValue("a"), std::dynamic_pointer_cast<ConfigBase>(stringList->at(0)));
    checkEquals(stringValue("b"), std::dynamic_pointer_cast<ConfigBase>(stringList->at(1)));
    checkEquals(stringValue("c"), std::dynamic_pointer_cast<ConfigBase>(stringList->at(2)));
}

TEST_F(ConfigTest, test01Exceptions) {
    auto conf = Config::load(resourcePath() + "/test01");

    // should throw Missing if key doesn't exist
    EXPECT_THROW(conf->getInt("doesnotexist"), ConfigExceptionMissing);

    // should throw Null if key is null
    EXPECT_THROW(conf->getInt("nulls.null"), ConfigExceptionNull);

    EXPECT_THROW(conf->getIntList("nulls.null"), ConfigExceptionNull);

    EXPECT_THROW(conf->getMilliseconds("nulls.null"), ConfigExceptionNull);

    EXPECT_THROW(conf->getNanoseconds("nulls.null"), ConfigExceptionNull);

    EXPECT_THROW(conf->getBytes("nulls.null"), ConfigExceptionNull);

    // should throw WrongType if key is wrong type and not convertible
    EXPECT_THROW(conf->getInt("booleans.trueAgain"), ConfigExceptionWrongType);

    EXPECT_THROW(conf->getBooleanList("arrays.ofInt"), ConfigExceptionWrongType);

    EXPECT_THROW(conf->getIntList("arrays.ofBoolean"), ConfigExceptionWrongType);

    EXPECT_THROW(conf->getObjectList("arrays.ofInt"), ConfigExceptionWrongType);

    EXPECT_THROW(conf->getMilliseconds("ints"), ConfigExceptionWrongType);

    EXPECT_THROW(conf->getNanoseconds("ints"), ConfigExceptionWrongType);

    EXPECT_THROW(conf->getBytes("ints"), ConfigExceptionWrongType);

    // should throw BadPath on various bad paths
    EXPECT_THROW(conf->getInt(".bad"), ConfigExceptionBadPath);

    EXPECT_THROW(conf->getInt("bad."), ConfigExceptionBadPath);

    EXPECT_THROW(conf->getInt("bad..bad"), ConfigExceptionBadPath);

    // should throw BadValue on things that don't parse
    // as durations and sizes
    EXPECT_THROW(conf->getMilliseconds("strings.a"), ConfigExceptionBadValue);

    EXPECT_THROW(conf->getNanoseconds("strings.a"), ConfigExceptionBadValue);

    EXPECT_THROW(conf->getBytes("strings.a"), ConfigExceptionBadValue);
}

TEST_F(ConfigTest, test01Conversions) {
    auto conf = Config::load(resourcePath() + "/test01");

    // should convert numbers to string
    EXPECT_EQ("42", conf->getString("ints.fortyTwo"));
    EXPECT_EQ("42.1", conf->getString("floats.fortyTwoPointOne"));

    // should convert string to number
    EXPECT_EQ(57, conf->getInt("strings.number"));
    EXPECT_DOUBLE_EQ(3.14, conf->getDouble("strings.double"));

    // should convert strings to boolean
    EXPECT_TRUE(conf->getBoolean("strings.true"));
    EXPECT_TRUE(conf->getBoolean("strings.yes"));
    EXPECT_FALSE(conf->getBoolean("strings.false"));
    EXPECT_FALSE(conf->getBoolean("strings.no"));

    // converting some random string to boolean fails though
    EXPECT_THROW(conf->getBoolean("strings.abcd"), ConfigExceptionWrongType);

    // should not convert strings to object or list
    EXPECT_THROW(conf->getObject("strings.a"), ConfigExceptionWrongType);

    EXPECT_THROW(conf->getList("strings.a"), ConfigExceptionWrongType);

    // should not convert object or list to string
    EXPECT_THROW(conf->getString("ints"), ConfigExceptionWrongType);

    EXPECT_THROW(conf->getString("arrays.ofInt"), ConfigExceptionWrongType);

    // should get durations
    EXPECT_EQ(1000LL, conf->getMilliseconds("durations.second"));
    EXPECT_EQ(1000000000LL, conf->getNanoseconds("durations.second"));
    EXPECT_EQ(1000LL, conf->getMilliseconds("durations.secondAsNumber"));
    EXPECT_EQ(1000000000LL, conf->getNanoseconds("durations.secondAsNumber"));
    EXPECT_TRUE(VectorInt64({1000LL, 2000LL, 3000LL, 4000LL}) ==
        conf->getMillisecondsList("durations.secondsList"));
    EXPECT_TRUE(VectorInt64({1000000000LL, 2000000000LL, 3000000000LL, 4000000000LL}) ==
        conf->getNanosecondsList("durations.secondsList"));
    EXPECT_EQ(500LL, conf->getMilliseconds("durations.halfSecond"));

    // should get size in bytes
    EXPECT_EQ(1024LL * 1024LL, conf->getBytes("memsizes.meg"));
    EXPECT_EQ(1024LL * 1024LL, conf->getBytes("memsizes.megAsNumber"));
    EXPECT_TRUE(VectorInt64({1024LL * 1024LL, 1024LL * 1024LL, 1024LL * 1024L}) ==
        conf->getBytesList("memsizes.megsList"));
    EXPECT_EQ(512LL * 1024LL, conf->getBytes("memsizes.halfMeg"));
}

TEST_F(ConfigTest, test01MergingOtherFormats) {
    auto conf = Config::load(resourcePath() + "/test01");

    // should have loaded stuff from .json
    EXPECT_EQ(1, conf->getInt("fromJson1"));
    EXPECT_EQ("A", conf->getString("fromJsonA"));
}

TEST_F(ConfigTest, test01ToString) {
    auto conf = Config::load(resourcePath() + "/test01");

    // toString() on conf objects doesn't throw (toString is just a debug string so not testing its result)
    std::dynamic_pointer_cast<ConfigBase>(conf)->toString();
}

TEST_F(ConfigTest, test01SystemFallbacks) {
    auto conf = Config::load(resourcePath() + "/test01");

    auto home = getenv("HOME");
    if (home) {
        EXPECT_EQ(home, conf->getString("system.home"));
    }
    else {
        checkEquals(nullValue(), std::dynamic_pointer_cast<ConfigBase>(conf->getObject("system")->get("home")));
    }
}

TEST_F(ConfigTest, test01Origins) {
    auto conf = Config::load(resourcePath() + "/test01");

    auto o1 = conf->getValue("ints.fortyTwo")->origin();
    EXPECT_TRUE(boost::ends_with(o1->description(), "test01.conf: 3"));
    EXPECT_TRUE(boost::ends_with(o1->filename(), "test01.conf"));
    EXPECT_EQ(3, o1->lineNumber());

    auto o2 = conf->getValue("fromJson1")->origin();
    EXPECT_TRUE(boost::ends_with(o2->description(), "test01.json: 2"));
    EXPECT_TRUE(boost::ends_with(o2->filename(), "test01.json"));
    EXPECT_EQ(2, o2->lineNumber());
}

TEST_F(ConfigTest, test01EntrySet) {
    auto conf = Config::load(resourcePath() + "/test01");

    auto entries = conf->entrySet();
    MapConfigValue map(entries.begin(), entries.end());
    checkEquals(intValue(42), std::dynamic_pointer_cast<ConfigBase>(map["ints.fortyTwo"]));
    EXPECT_EQ(0, map.count("nulls.null"));
}

TEST_F(ConfigTest, test02SubstitutionsWithWeirdPaths) {
    auto conf = Config::load(resourcePath() + "/test02");

    EXPECT_EQ(42, conf->getInt("42_a"));
    EXPECT_EQ(42, conf->getInt("42_b"));
    EXPECT_EQ(57, conf->getInt("57_a"));
    EXPECT_EQ(57, conf->getInt("57_b"));
    EXPECT_EQ(103, conf->getInt("103_a"));
}

TEST_F(ConfigTest, test02UseWeirdPathsWithConfigObject) {
    auto conf = Config::load(resourcePath() + "/test02");

    // we're checking that the getters in ConfigObject support
    // these weird path expressions
    EXPECT_EQ(42, conf->getInt(" \"\".\"\".\"\" "));
    EXPECT_EQ(57, conf->getInt("a.b.c"));
    EXPECT_EQ(57, conf->getInt(" \"a\".\"b\".\"c\" "));
    EXPECT_EQ(103, conf->getInt(" \"a.b.c\" "));
}

TEST_F(ConfigTest, test03Includes) {
    auto conf = Config::load(resourcePath() + "/test03");

    // include should have overridden the "ints" value in test03
    EXPECT_EQ(42, conf->getInt("test01.ints.fortyTwo"));
    // include should have been overridden by 42
    EXPECT_EQ(42, conf->getInt("test01.booleans"));
    EXPECT_EQ(42, conf->getInt("test01.booleans"));
    // include should have gotten .json also
    EXPECT_EQ("A", conf->getString("test01.fromJsonA"));
    // test02 was included
    EXPECT_EQ(57, conf->getInt("test02.a.b.c"));
    // equiv01/original.json was included (it has a slash in the name)
    EXPECT_EQ("a", conf->getString("equiv01.strings.a"));

    // Now check that substitutions still work
    EXPECT_EQ(42, conf->getInt("test01.ints.fortyTwoAgain"));
    EXPECT_TRUE(VectorString({"a", "b", "c"}) == conf->getStringList("test01.arrays.ofString"));
    EXPECT_EQ(103, conf->getInt("test02.103_a"));

    // and system fallbacks still work
    auto home = getenv("HOME");
    if (home) {
        EXPECT_EQ(home, conf->getString("test01.system.home"));
    }
    else {
        checkEquals(nullValue(), std::dynamic_pointer_cast<ConfigBase>(conf->getObject("test01.system")->get("home")));
    }
    std::string concatenated = conf->getString("test01.system.concatenated");
    EXPECT_TRUE(boost::contains(concatenated, "Your Java version"));

    // check that includes into the root object work and that
    // "substitutions look relative-to-included-file first then at root second" works
    EXPECT_EQ("This is in the included file", conf->getString("a"));
    EXPECT_EQ("This is in the including file", conf->getString("b"));
    EXPECT_EQ("This is in the included file", conf->getString("subtree.a"));
    EXPECT_EQ("This is in the including file", conf->getString("subtree.b"));
}

TEST_F(ConfigTest, test04LoadAkkaReference) {
    auto conf = Config::load(resourcePath() + "/test04");

    // Note, test04 is an unmodified old-style akka.conf,
    // which means it has an outer akka{} namespace.
    // that namespace wouldn't normally be used with
    // this library because the conf object is not global,
    // it's per-module already.
    EXPECT_EQ("2.0-SNAPSHOT", conf->getString("akka.version"));
    EXPECT_EQ(8, conf->getInt("akka.event-handler-dispatcher.max-pool-size"));
    EXPECT_EQ("round-robin", conf->getString("akka.actor.deployment.\"/app/service-ping\".router"));
    EXPECT_EQ(true, conf->getBoolean("akka.stm.quick-release"));
}

TEST_F(ConfigTest, test05LoadPlayApplicationConf) {
    auto conf = Config::load(resourcePath() + "/test05");

    EXPECT_EQ("prod", conf->getString("%prod.application.mode"));
    EXPECT_EQ("Yet another blog", conf->getString("blog.title"));
}

TEST_F(ConfigTest, test06Merge) {
    // test06 mostly exists because its render() round trip is tricky
    auto conf = Config::load(resourcePath() + "/test06");

    EXPECT_EQ(2, conf->getInt("x"));
    EXPECT_EQ(10, conf->getInt("y.foo"));
    EXPECT_EQ("world", conf->getString("y.hello"));
}

// test07IncludingResourcesFromFiles - skipped
// test08IncludingSlashPrefixedResources - skipped

TEST_F(ConfigTest, test09DelayedMerge) {
    auto conf = Config::parseFile(resourcePath() + "/test09.conf");

    EXPECT_TRUE(instanceof<ConfigDelayedMergeObject>(conf->root()->get("a")));
    EXPECT_TRUE(instanceof<ConfigDelayedMerge>(conf->root()->get("b")));

    // a.c should work without resolving because no more merging is needed to compute it
    EXPECT_EQ(3, conf->getInt("a.c"));

    EXPECT_THROW(conf->getInt("a.q"), ConfigExceptionNotResolved);

    // be sure resolving doesn't throw
    auto resolved = conf->resolve();
    EXPECT_EQ(3, resolved->getInt("a.c"));
    EXPECT_EQ(5, resolved->getInt("b"));
    EXPECT_EQ(10, resolved->getInt("a.q"));
}

TEST_F(ConfigTest, test10DelayedMergeRelativizing) {
    auto conf = Config::parseFile(resourcePath() + "/test10.conf");
    auto resolved = conf->resolve();
    EXPECT_EQ(3, resolved->getInt("foo.a.c"));
    EXPECT_EQ(5, resolved->getInt("foo.b"));
    EXPECT_EQ(10, resolved->getInt("foo.a.q"));
}

TEST_F(ConfigTest, renderRoundTrip) {
    auto optionsCombos = {
        ConfigRenderOptions::defaults()->setFormatted(false)->setOriginComments(false)->setComments(false),
        ConfigRenderOptions::defaults()->setFormatted(false)->setOriginComments(false)->setComments(true),
        ConfigRenderOptions::defaults()->setFormatted(false)->setOriginComments(true)->setComments(false),
        ConfigRenderOptions::defaults()->setFormatted(false)->setOriginComments(true)->setComments(true),
        ConfigRenderOptions::defaults()->setFormatted(true)->setOriginComments(false)->setComments(false),
        ConfigRenderOptions::defaults()->setFormatted(true)->setOriginComments(false)->setComments(true),
        ConfigRenderOptions::defaults()->setFormatted(true)->setOriginComments(true)->setComments(false),
        ConfigRenderOptions::defaults()->setFormatted(true)->setOriginComments(true)->setComments(true)
    };

    for (uint32_t i = 1; i <= 10; ++i) {
        std::ostringstream name;
        name << "/test" << std::setfill('0') << std::setw(2) << i;
        auto conf = Config::parseFileAnySyntax(resourcePath() + name.str(), ConfigParseOptions::defaults()->setAllowMissing(false));
        for (auto& renderOptions : optionsCombos) {
            auto unresolvedRender = conf->root()->render(renderOptions);
            auto resolved = conf->resolve();
            auto resolvedRender = resolved->root()->render(renderOptions);

            checkEquals(std::dynamic_pointer_cast<ConfigBase>(conf->root()), std::dynamic_pointer_cast<ConfigBase>(Config::parseString(unresolvedRender, ConfigParseOptions::defaults())->root()));
            checkEquals(std::dynamic_pointer_cast<ConfigBase>(resolved->root()), std::dynamic_pointer_cast<ConfigBase>(Config::parseString(resolvedRender, ConfigParseOptions::defaults())->root()));

            if (!(renderOptions->getComments() || renderOptions->getOriginComments())) {
                // should get valid JSON if we don't have comments and are resolved
                Config::parseString(resolvedRender, ConfigParseOptions::defaults()->setSyntax(ConfigSyntax::JSON));
            }
        }
    }
}
