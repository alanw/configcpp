/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "test_fixture.h"
#include "configcpp/detail/resolve_context.h"
#include "configcpp/detail/abstract_config_object.h"
#include "configcpp/detail/config_reference.h"
#include "configcpp/detail/config_delayed_merge_object.h"
#include "configcpp/detail/config_delayed_merge.h"
#include "configcpp/detail/simple_config_object.h"
#include "configcpp/detail/simple_config_origin.h"
#include "configcpp/detail/path.h"
#include "configcpp/config.h"
#include "configcpp/config_list.h"
#include "configcpp/config_object.h"
#include "configcpp/config_exception.h"
#include "configcpp/config_resolve_options.h"

using namespace config;

class ConfigSubstitutionTest : public TestFixture {
protected:
    ConfigPtr resolveWithoutFallbacks(const AbstractConfigObjectPtr& v) {
        auto options = ConfigResolveOptions::noSystem();
        return std::static_pointer_cast<AbstractConfigObject>(ResolveContext::resolve(std::static_pointer_cast<AbstractConfigValue>(v), v, options))->toConfig();
    }

    AbstractConfigValuePtr resolveWithoutFallbacks(const AbstractConfigValuePtr& s, const AbstractConfigObjectPtr& root) {
        auto options = ConfigResolveOptions::noSystem();
        return ResolveContext::resolve(s, root, options);
    }

    ConfigPtr resolve(const AbstractConfigObjectPtr& v) {
        auto options = ConfigResolveOptions::defaults();
        return std::static_pointer_cast<AbstractConfigObject>(ResolveContext::resolve(std::static_pointer_cast<AbstractConfigValue>(v), v, options))->toConfig();
    }

    AbstractConfigValuePtr resolve(const AbstractConfigValuePtr& s, const AbstractConfigObjectPtr& root) {
        auto options = ConfigResolveOptions::defaults();
        return ResolveContext::resolve(s, root, options);
    }

    AbstractConfigObjectPtr simpleObject() {
        return parseObject("{\n\"foo\" : 42,\n\"bar\" : {\n\"int\" : 43,\n\"bool\" : true,\n\"null\" : null,\n\"string\" : \"hello\",\n\"double\" : 3.14}}");
    }

    AbstractConfigObjectPtr substChainObject() {
        return parseObject("{\n\"foo\" : ${bar},\"bar\" : ${a.b.c},\n\"a\" : { \"b\" : { \"c\" : 57 } }\n}");
    }

    AbstractConfigObjectPtr substCycleObject() {
        return parseObject("{\n\"foo\" : ${bar},\n\"bar\" : ${a.b.c}\n\"a\" : { \"b\" : { \"c\" : ${foo} } }}");
    }

    AbstractConfigObjectPtr substCycleObjectOptionalLink() {
        // ALL the links have to be optional here for the cycle to be ignored
        return parseObject("{\n\"foo\" : ${?bar},\n\"bar\" : ${?a.b.c},\n\"a\" : { \"b\" : { \"c\" : ${?foo} } }\n}");
    }

    AbstractConfigObjectPtr substSideEffectCycle() {
        return parseObject("{\n\"foo\" : ${a.b.c},\n\"a\" : { \"b\" : { \"c\" : 42, \"cycle\" : ${foo} }, \"cycle\" : ${foo} }\n}");
    }

    AbstractConfigObjectPtr delayedMergeObjectResolveProblem1() {
        return parseObject("defaults {\n   a = 1\n   b = 2\n  }\n"
                           "// make item1 into a ConfigDelayedMergeObject\n"
                           "item1 = ${defaults}\n"
                           "// note that we'll resolve to a non-object value\n"
                           "// so item1.b will ignoreFallbacks and not depend on\n"
                           "// ${defaults}\n"
                           "item1.b = 3\n"
                           "// be sure we can resolve a substitution to a value in\n"
                           "// a delayed-merge object.\n"
                           "item2.b = ${item1.b}");
    }

    AbstractConfigObjectPtr delayedMergeObjectResolveProblem2() {
        return parseObject("defaults {\n   a = 1\n   b = 2\n  }\n"
                           "// make item1 into a ConfigDelayedMergeObject\n"
                           "item1 = ${defaults}\n"
                           "// note that we'll resolve to an object value\n"
                           "// so item1.b will depend on also looking up ${defaults}\n"
                           "item1.b = { c : 43 }\n"
                           "// be sure we can resolve a substitution to a value in\n"
                           "// a delayed-merge object.\n"
                           "item2.b = ${item1.b}");
    }

    AbstractConfigObjectPtr delayedMergeObjectResolveProblem3() {
        return parseObject("item1.b.c = 100\n"
                           "defaults {\n"
                           " // we depend on item1.b.c\n"
                           " a = ${item1.b.c}\n"
                           " b = 2\n"
                           "}\n"
                           "// make item1 into a ConfigDelayedMergeObject\n"
                           "item1 = ${defaults}\n"
                           "// the ${item1.b.c} above in ${defaults} should ignore\n"
                           "// this because it only looks back\n"
                           "item1.b = { c : 43 }\n"
                           "// be sure we can resolve a substitution to a value in\n"
                           "// a delayed-merge object.\n"
                           "item2.b = ${item1.b}");
    }

    AbstractConfigObjectPtr delayedMergeObjectResolveProblem4() {
        return parseObject("defaults {\n"
                           " a = 1\n"
                           " b = 2\n"
                           "}\n"
                           "item1.b = 7\n"
                           "// make item1 into a ConfigDelayedMerge\n"
                           "item1 = ${defaults}\n"
                           "// be sure we can resolve a substitution to a value in\n"
                           "// a delayed-merge object.\n"
                           "item2.b = ${item1.b}");
    }

    AbstractConfigObjectPtr delayedMergeObjectResolveProblem5() {
        return parseObject("defaults {\n"
                           " a = ${item1.b} // tricky cycle\n"
                           " b = 2\n"
                           "}\n"
                           "item1.b = 7\n"
                           "// make item1 into a ConfigDelayedMerge\n"
                           "item1 = ${defaults}\n"
                           "// be sure we can resolve a substitution to a value in\n"
                           "// a delayed-merge object.\n"
                           "item2.b = ${item1.b}");
    }

    AbstractConfigObjectPtr delayedMergeObjectResolveProblem6() {
        return parseObject("z = 15\n"
                           "defaults-defaults-defaults {\n"
                           " m = ${z}\n"
                           " n.o.p = ${z}\n"
                           "}\n"
                           "defaults-defaults {\n"
                           " x = 10\n"
                           " y = 11\n"
                           " asdf = ${z}\n"
                           "}\n"
                           "defaults {\n"
                           " a = 1\n"
                           " b = 2\n"
                           "}\n"
                           "defaults-alias = ${defaults}\n"
                           "// make item1 into a ConfigDelayedMergeObject several layers deep\n"
                           "// that will NOT become resolved just because we resolve one path\n"
                           "// through it.\n"
                           "item1 = 345\n"
                           "item1 = ${?NONEXISTENT}\n"
                           "item1 = ${defaults-defaults-defaults}\n"
                           "item1 = {}\n"
                           "item1 = ${defaults-defaults}\n"
                           "item1 = ${defaults-alias}\n"
                           "item1 = ${defaults}\n"
                           "item1.b = { c : 43 }\n"
                           "item1.xyz = 101\n"
                           "// be sure we can resolve a substitution to a value in\n"
                           "// a delayed-merge object.\n"
                           "item2.b = ${item1.b}");
    }

    AbstractConfigObjectPtr delayedMergeObjectWithKnownValue() {
        return parseObject("defaults {\n"
                           " a = 1\n"
                           " b = 2\n"
                           "}\n"
                           "// make item1 into a ConfigDelayedMergeObject\n"
                           "item1 = ${defaults}\n"
                           "// note that we'll resolve to a non-object value\n"
                           "// so item1.b will ignoreFallbacks and not depend on\n"
                           "// ${defaults}\n"
                           "item1.b = 3");
    }

    AbstractConfigObjectPtr delayedMergeObjectNeedsFullResolve() {
        return parseObject("defaults {\n"
                           " a = 1\n"
                           " b = { c : 31 }\n"
                           "}\n"
                           "item1 = ${defaults}\n"
                           "// because b is an object, fetching it requires resolving ${defaults} above\n"
                           "// to see if there are more keys to merge with b.\n"
                           "item1.b = { c : 41 }");
    }

    AbstractConfigObjectPtr delayedMergeObjectEmbrace() {
        // objects that mutually refer to each other
        return parseObject("defaults {\n"
                           " a = 1\n"
                           " b = 2\n"
                           "}\n"
                           "item1 = ${defaults}\n"
                           "// item1.c refers to a field in item2 that refers to item1\n"
                           "item1.c = ${item2.d}\n"
                           "// item1.x refers to a field in item2 that doesn't go back to item1\n"
                           "item1.x = ${item2.y}\n"
                           "item2 = ${defaults}\n"
                           "// item2.d refers to a field in item1\n"
                           "item2.d = ${item1.a}\n"
                           "item2.y = 15");
    }

    AbstractConfigObjectPtr plainObjectEmbrace() {
        // objects that mutually refer to each other
        return parseObject("item1.a = 10\n"
                           "item1.b = ${item2.d}\n"
                           "item2.c = 12\n"
                           "item2.d = 14\n"
                           "item2.e = ${item1.a}\n"
                           "item2.f = ${item1.b}   // item1.b goes back to item2\n"
                           "item2.g = ${item2.f}   // goes back to ourselves");
    }

    AbstractConfigObjectPtr substComplexObject() {
        return parseObject("{\n"
                           "\"foo\" : ${bar},\n"
                           "\"bar\" : ${a.b.c},\n"
                           "\"a\" : { \"b\" : { \"c\" : 57, \"d\" : ${foo}, \"e\" : { \"f\" : ${foo} } } },\n"
                           "\"objA\" : ${a},\n"
                           "\"objB\" : ${a.b},\n"
                           "\"objE\" : ${a.b.e},\n"
                           "\"foo.bar\" : 37,\n"
                           "\"arr\" : [ ${foo}, ${a.b.c}, ${\"foo.bar\"}, ${objB.d}, ${objA.b.e.f}, ${objE.f} ],\n"
                           "\"ptrToArr\" : ${arr},\n"
                           "\"x\" : { \"y\" : { \"ptrToPtrToArr\" : ${ptrToArr} } }"
                           "}");
    }

    AbstractConfigObjectPtr substEnvVarObject() {
        return parseObject("{\n"
                           "\"home\" : ${?HOME},\n"
                           "\"pwd\" : ${?PWD},\n"
                           "\"shell\" : ${?SHELL},\n"
                           "\"lang\" : ${?LANG},\n"
                           "\"path\" : ${?PATH},\n"
                           "\"not_here\" : ${?NOT_HERE}"
                           "}");
    }
};

TEST_F(ConfigSubstitutionTest, resolveTrivialKey) {
    auto s = subst("foo");
    auto v = resolveWithoutFallbacks(s, simpleObject());
    checkEquals(intValue(42), v);
}

TEST_F(ConfigSubstitutionTest, resolveTrivialPath) {
    auto s = subst("bar.int");
    auto v = resolveWithoutFallbacks(s, simpleObject());
    checkEquals(intValue(43), v);
}

TEST_F(ConfigSubstitutionTest, resolveInt) {
    auto s = subst("bar.int");
    auto v = resolveWithoutFallbacks(s, simpleObject());
    checkEquals(intValue(43), v);
}

TEST_F(ConfigSubstitutionTest, resolveBool) {
    auto s = subst("bar.bool");
    auto v = resolveWithoutFallbacks(s, simpleObject());
    checkEquals(boolValue(true), v);
}

TEST_F(ConfigSubstitutionTest, resolveNull) {
    auto s = subst("bar.null");
    auto v = resolveWithoutFallbacks(s, simpleObject());
    checkEquals(nullValue(), v);
}

TEST_F(ConfigSubstitutionTest, resolveString) {
    auto s = subst("bar.string");
    auto v = resolveWithoutFallbacks(s, simpleObject());
    checkEquals(stringValue("hello"), v);
}

TEST_F(ConfigSubstitutionTest, resolveDouble) {
    auto s = subst("bar.double");
    auto v = resolveWithoutFallbacks(s, simpleObject());
    checkEquals(doubleValue(3.14), v);
}

TEST_F(ConfigSubstitutionTest, resolveMissingThrows) {
    try {
        auto s = subst("bar.missing");
        auto v = resolveWithoutFallbacks(s, simpleObject());
        FAIL() << "expected: ConfigExceptionUnresolvedSubstitution";
    }
    catch (ConfigExceptionUnresolvedSubstitution& e) {
        EXPECT_FALSE(boost::contains(e.what(), "cycle"));
    }
}

TEST_F(ConfigSubstitutionTest, resolveIntInString) {
    auto s = substInString("bar.int");
    auto v = resolveWithoutFallbacks(s, simpleObject());
    checkEquals(stringValue("start<43>end"), v);
}

TEST_F(ConfigSubstitutionTest, resolveNullInString) {
    auto s = substInString("bar.null");
    auto v = resolveWithoutFallbacks(s, simpleObject());
    checkEquals(stringValue("start<null>end"), v);
}

TEST_F(ConfigSubstitutionTest, resolveMissingInString) {
    auto s = substInString("bar.missing", true);
    auto v = resolveWithoutFallbacks(s, simpleObject());
    // absent object becomes empty string
    checkEquals(stringValue("start<>end"), v);

    try {
        auto s2 = substInString("bar.missing", false);
        resolveWithoutFallbacks(s2, simpleObject());
        FAIL() << "expected: ConfigExceptionUnresolvedSubstitution";
    }
    catch (ConfigExceptionUnresolvedSubstitution& e) {
    }
}

TEST_F(ConfigSubstitutionTest, resolveBoolInString) {
    auto s = substInString("bar.bool");
    auto v = resolveWithoutFallbacks(s, simpleObject());
    checkEquals(stringValue("start<true>end"), v);
}

TEST_F(ConfigSubstitutionTest, resolveStringInString) {
    auto s = substInString("bar.string");
    auto v = resolveWithoutFallbacks(s, simpleObject());
    checkEquals(stringValue("start<hello>end"), v);
}

TEST_F(ConfigSubstitutionTest, resolveDoubleInString) {
    auto s = substInString("bar.double");
    auto v = resolveWithoutFallbacks(s, simpleObject());
    checkEquals(stringValue("start<3.14>end"), v);
}

TEST_F(ConfigSubstitutionTest, missingInArray) {
    auto obj = parseObject("\na : ${?missing}, b : ${?also.missing}, c : ${?b}, d : ${?c}\n");
    auto resolved = resolve(obj);
    EXPECT_TRUE(resolved->empty());
}

TEST_F(ConfigSubstitutionTest, chainSubstitutions) {
    auto s = subst("foo");
    auto v = resolveWithoutFallbacks(s, substChainObject());
    checkEquals(intValue(57), v);
}

TEST_F(ConfigSubstitutionTest, substitutionsLookForward) {
    auto obj = parseObject("a=1,b=${a},a=2");
    auto resolved = resolve(obj);
    EXPECT_EQ(2, resolved->getInt("b"));
}

TEST_F(ConfigSubstitutionTest, throwOnCycles) {
    try {
        auto s = subst("foo");
        auto v = resolveWithoutFallbacks(s, substCycleObject());
        FAIL() << "expected: ConfigExceptionUnresolvedSubstitution";
    }
    catch (ConfigExceptionUnresolvedSubstitution& e) {
        EXPECT_TRUE(boost::contains(e.what(), "cycle"));
        EXPECT_TRUE(boost::contains(e.what(), "${foo}, ${bar}, ${a.b.c}, ${foo}"));
    }
}

TEST_F(ConfigSubstitutionTest, throwOnOptionalReferenceToNonOptionalCycle) {
    // we look up ${?foo}, but the cycle has hard
    // non-optional links in it so still has to throw.
    try {
        auto s = subst("foo", true);
        auto v = resolveWithoutFallbacks(s, substCycleObject());
        FAIL() << "expected: ConfigExceptionUnresolvedSubstitution";
    }
    catch (ConfigExceptionUnresolvedSubstitution& e) {
        EXPECT_TRUE(boost::contains(e.what(), "cycle"));
    }
}

TEST_F(ConfigSubstitutionTest, optionalLinkCyclesActLikeUndefined) {
    auto s = subst("foo", true);
    auto v = resolveWithoutFallbacks(s, substCycleObjectOptionalLink());
    EXPECT_FALSE(v) << "Cycle with optional links in it resolves to null if it's a cycle";
}

TEST_F(ConfigSubstitutionTest, throwOnTwoKeyCycle) {
    try {
        auto obj = parseObject("a:${b},b:${a}");
        resolve(obj);
        FAIL() << "expected: ConfigExceptionUnresolvedSubstitution";
    }
    catch (ConfigExceptionUnresolvedSubstitution& e) {
        EXPECT_TRUE(boost::contains(e.what(), "cycle"));
    }
}

TEST_F(ConfigSubstitutionTest, throwOnFourKeyCycle) {
    try {
        auto obj = parseObject("a:${b},b:${c},c:${d},d:${a}");
        resolve(obj);
        FAIL() << "expected: ConfigExceptionUnresolvedSubstitution";
    }
    catch (ConfigExceptionUnresolvedSubstitution& e) {
        EXPECT_TRUE(boost::contains(e.what(), "cycle"));
    }
}

TEST_F(ConfigSubstitutionTest, resolveObject) {
    auto resolved = resolveWithoutFallbacks(substChainObject());
    EXPECT_EQ(57, resolved->getInt("foo"));
    EXPECT_EQ(57, resolved->getInt("bar"));
    EXPECT_EQ(57, resolved->getInt("a.b.c"));
}

TEST_F(ConfigSubstitutionTest, avoidSideEffectCycles) {
    // The point of this test is that in traversing objects
    // to resolve a path, we need to avoid resolving
    // substitutions that are in the traversed objects but
    // are not directly required to resolve the path.
    // i.e. there should not be a cycle in this test.
    auto resolved = resolveWithoutFallbacks(substSideEffectCycle());
    EXPECT_EQ(42, resolved->getInt("foo"));
    EXPECT_EQ(42, resolved->getInt("a.b.cycle"));
    EXPECT_EQ(42, resolved->getInt("a.cycle"));
}

TEST_F(ConfigSubstitutionTest, ignoreHiddenUndefinedSubst) {
    // if a substitution is overridden then it shouldn't matter that it's undefined
    auto obj = parseObject("a=${nonexistent},a=42");
    auto resolved = resolve(obj);
    EXPECT_EQ(42, resolved->getInt("a"));
}

TEST_F(ConfigSubstitutionTest, objectDoesNotHideUndefinedSubst) {
    // if a substitution is overridden by an object we still need to
    // evaluate the substitution
    try {
        auto obj = parseObject("a=${nonexistent},a={ b : 42 }");
        resolve(obj);
        FAIL() << "expected: ConfigExceptionUnresolvedSubstitution";
    }
    catch (ConfigExceptionUnresolvedSubstitution& e) {
        EXPECT_TRUE(boost::contains(e.what(), "Could not resolve"));
    }
}

TEST_F(ConfigSubstitutionTest, ignoreHiddenCircularSubst) {
    // if a substitution is overridden then it shouldn't matter that it's circular
    auto obj = parseObject("a=${a},a=42");
    auto resolved = resolve(obj);
    EXPECT_EQ(42, resolved->getInt("a"));
}

TEST_F(ConfigSubstitutionTest, avoidDelayedMergeObjectResolveProblem1) {
    auto obj = delayedMergeObjectResolveProblem1();
    EXPECT_TRUE(instanceof<ConfigDelayedMergeObject>(obj->attemptPeekWithPartialResolve("item1")));

    auto resolved = resolveWithoutFallbacks(delayedMergeObjectResolveProblem1());

    EXPECT_EQ(3, resolved->getInt("item1.b"));
    EXPECT_EQ(3, resolved->getInt("item2.b"));
}

TEST_F(ConfigSubstitutionTest, avoidDelayedMergeObjectResolveProblem2) {
    auto obj = delayedMergeObjectResolveProblem2();
    EXPECT_TRUE(instanceof<ConfigDelayedMergeObject>(obj->attemptPeekWithPartialResolve("item1")));

    auto resolved = resolveWithoutFallbacks(delayedMergeObjectResolveProblem2());

    checkEquals(parseObject("{ c : 43 }"), std::dynamic_pointer_cast<AbstractConfigObject>(resolved->getObject("item1.b")));
    EXPECT_EQ(43, resolved->getInt("item1.b.c"));
    EXPECT_EQ(43, resolved->getInt("item2.b.c"));
}

TEST_F(ConfigSubstitutionTest, avoidDelayedMergeObjectResolveProblem3) {
    auto obj = delayedMergeObjectResolveProblem3();
    EXPECT_TRUE(instanceof<ConfigDelayedMergeObject>(obj->attemptPeekWithPartialResolve("item1")));

    auto resolved = resolveWithoutFallbacks(delayedMergeObjectResolveProblem3());

    checkEquals(parseObject("{ c : 43 }"), std::dynamic_pointer_cast<AbstractConfigObject>(resolved->getObject("item1.b")));
    EXPECT_EQ(43, resolved->getInt("item1.b.c"));
    EXPECT_EQ(43, resolved->getInt("item2.b.c"));
    EXPECT_EQ(100, resolved->getInt("defaults.a"));
}

TEST_F(ConfigSubstitutionTest, avoidDelayedMergeObjectResolveProblem4) {
    // in this case we have a ConfigDelayedMerge not a ConfigDelayedMergeObject
    auto obj = delayedMergeObjectResolveProblem4();
    EXPECT_TRUE(instanceof<ConfigDelayedMerge>(obj->attemptPeekWithPartialResolve("item1")));

    auto resolved = resolveWithoutFallbacks(delayedMergeObjectResolveProblem4());

    EXPECT_EQ(2, resolved->getInt("item1.b"));
    EXPECT_EQ(2, resolved->getInt("item2.b"));
}

TEST_F(ConfigSubstitutionTest, avoidDelayedMergeObjectResolveProblem5) {
    // in this case we have a ConfigDelayedMerge not a ConfigDelayedMergeObject
    auto obj = delayedMergeObjectResolveProblem5();
    EXPECT_TRUE(instanceof<ConfigDelayedMerge>(obj->attemptPeekWithPartialResolve("item1")));

    auto resolved = resolveWithoutFallbacks(delayedMergeObjectResolveProblem5());

    EXPECT_EQ(2, resolved->getInt("item1.b"));
    EXPECT_EQ(2, resolved->getInt("item2.b"));
    EXPECT_EQ(2, resolved->getInt("defaults.a"));
}

TEST_F(ConfigSubstitutionTest, avoidDelayedMergeObjectResolveProblem6) {
    auto obj = delayedMergeObjectResolveProblem6();
    EXPECT_TRUE(instanceof<ConfigDelayedMergeObject>(obj->attemptPeekWithPartialResolve("item1")));

    // should be able to attemptPeekWithPartialResolve() a known non-object without resolving
    EXPECT_EQ(101, std::dynamic_pointer_cast<AbstractConfigObject>(delayedMergeObjectResolveProblem6()->toConfig()->getObject("item1"))->attemptPeekWithPartialResolve("xyz")->unwrapped<int32_t>());

    auto resolved = resolveWithoutFallbacks(delayedMergeObjectResolveProblem6());

    checkEquals(parseObject("{ c : 43 }"), std::dynamic_pointer_cast<AbstractConfigObject>(resolved->getObject("item1.b")));
    EXPECT_EQ(43, resolved->getInt("item1.b.c"));
    EXPECT_EQ(43, resolved->getInt("item2.b.c"));
    EXPECT_EQ(15, resolved->getInt("item1.n.o.p"));
}

TEST_F(ConfigSubstitutionTest, fetchKnownValueFromDelayedMergeObject) {
    auto obj = delayedMergeObjectWithKnownValue();
    EXPECT_TRUE(instanceof<ConfigDelayedMergeObject>(obj->attemptPeekWithPartialResolve("item1")));

    EXPECT_EQ(3, delayedMergeObjectWithKnownValue()->toConfig()->getConfig("item1")->getInt("b"));
}

TEST_F(ConfigSubstitutionTest, failToFetchFromDelayedMergeObjectNeedsFullResolve) {
    auto obj = delayedMergeObjectWithKnownValue();
    EXPECT_TRUE(instanceof<ConfigDelayedMergeObject>(obj->attemptPeekWithPartialResolve("item1")));

    try {
        delayedMergeObjectNeedsFullResolve()->toConfig()->getObject("item1.b");
        FAIL() << "expected: ConfigExceptionNotResolved";
    }
    catch (ConfigExceptionNotResolved& e) {
        EXPECT_TRUE(boost::contains(e.what(), "item1.b"));
    }
}

TEST_F(ConfigSubstitutionTest, resolveDelayedMergeObjectEmbrace) {
    auto obj = delayedMergeObjectEmbrace();
    EXPECT_TRUE(instanceof<ConfigDelayedMergeObject>(obj->attemptPeekWithPartialResolve("item1")));
    EXPECT_TRUE(instanceof<ConfigDelayedMergeObject>(obj->attemptPeekWithPartialResolve("item2")));

    auto resolved = delayedMergeObjectEmbrace()->toConfig()->resolve();

    EXPECT_EQ(1, resolved->getInt("item1.c"));
    EXPECT_EQ(1, resolved->getInt("item2.d"));
    EXPECT_EQ(15, resolved->getInt("item1.x"));
}

TEST_F(ConfigSubstitutionTest, resolvePlainObjectEmbrace) {
    auto obj = plainObjectEmbrace();
    EXPECT_TRUE(instanceof<SimpleConfigObject>(obj->attemptPeekWithPartialResolve("item1")));
    EXPECT_TRUE(instanceof<SimpleConfigObject>(obj->attemptPeekWithPartialResolve("item2")));

    auto resolved = plainObjectEmbrace()->toConfig()->resolve();

    EXPECT_EQ(14, resolved->getInt("item1.b"));
    EXPECT_EQ(10, resolved->getInt("item2.e"));
    EXPECT_EQ(14, resolved->getInt("item2.f"));
    EXPECT_EQ(14, resolved->getInt("item2.g"));
}

TEST_F(ConfigSubstitutionTest, useRelativeToSameFileWhenRelativized) {
    auto child = parseObject("foo=in child,bar=${foo}");

    MapAbstractConfigValue values;

    values["a"] = child->relativized(Path::make_instance(VectorString({"a"})));
    // this "foo" should NOT be used.
    values["foo"] = stringValue("in parent");

    auto resolved = resolve(SimpleConfigObject::make_instance(fakeOrigin(), values));

    EXPECT_EQ("in child", resolved->getString("a.bar"));
}

TEST_F(ConfigSubstitutionTest, useRelativeToRootWhenRelativized) {
    // here, "foo" is not defined in the child
    auto child = parseObject("bar=${foo}");

    MapAbstractConfigValue values;

    values["a"] = child->relativized(Path::make_instance(VectorString({"a"})));
    // so this "foo" SHOULD be used
    values["foo"] = stringValue("in parent");

    auto resolved = resolve(SimpleConfigObject::make_instance(fakeOrigin(), values));

    EXPECT_EQ("in parent", resolved->getString("a.bar"));
}

TEST_F(ConfigSubstitutionTest, complexResolve) {
    auto resolved = resolveWithoutFallbacks(substComplexObject());

    EXPECT_EQ(57, resolved->getInt("foo"));
    EXPECT_EQ(57, resolved->getInt("bar"));
    EXPECT_EQ(57, resolved->getInt("a.b.c"));
    EXPECT_EQ(57, resolved->getInt("a.b.d"));
    EXPECT_EQ(57, resolved->getInt("objB.d"));
    EXPECT_TRUE(VectorInt({57, 57, 37, 57, 57, 57}) == resolved->getIntList("arr"));
    EXPECT_TRUE(VectorInt({57, 57, 37, 57, 57, 57}) == resolved->getIntList("ptrToArr"));
    EXPECT_TRUE(VectorInt({57, 57, 37, 57, 57, 57}) == resolved->getIntList("x.y.ptrToPtrToArr"));
}

TEST_F(ConfigSubstitutionTest, fallbackToEnv) {
    auto resolved = resolve(substEnvVarObject());

    uint32_t existed = 0;
    for (auto& k : *resolved->root()) {
        auto e = getenv(boost::to_upper_copy(k.first).c_str());
        if (e) {
            existed += 1;
            EXPECT_EQ(e, resolved->getString(k.first));
        }
        else {
            EXPECT_EQ(0, resolved->root()->count(k.first));
        }
    }
    if (existed == 0) {
        FAIL() << "None of the env vars we tried to use for testing were set";
    }
}

TEST_F(ConfigSubstitutionTest, noFallbackToEnvIfValuesAreNull) {
    // create a fallback object with all the env var names
    // set to null. we want to be sure this blocks
    // lookup in the environment. i.e. if there is a
    // { HOME : null } then ${HOME} should be null.

    MapVariant nullsMap;
    for (auto& k : *substEnvVarObject()) {
        nullsMap[boost::to_upper_copy(k.first)] = null();
    }
    auto nulls = Config::parseMap(nullsMap, "nulls map");

    auto resolved = resolve(std::dynamic_pointer_cast<AbstractConfigObject>(substEnvVarObject()->withFallback(nulls)));

    for (auto& k : *resolved->root()) {
        ASSERT_TRUE(!!resolved->root()->get(k.first));
        checkEquals(nullValue(), std::dynamic_pointer_cast<AbstractConfigValue>(resolved->root()->get(k.first)));
    }
}

TEST_F(ConfigSubstitutionTest, fallbackToEnvWhenRelativized) {
    MapAbstractConfigValue values;

    values["a"] = substEnvVarObject()->relativized(Path::make_instance(VectorString({"a"})));

    auto resolved = resolve(SimpleConfigObject::make_instance(fakeOrigin(), values));

    uint32_t existed = 0;
    for (auto& k : *resolved->getObject("a")) {
        auto e = getenv(boost::to_upper_copy(k.first).c_str());
        if (e) {
            existed += 1;
            EXPECT_EQ(e, resolved->getConfig("a")->getString(k.first));
        }
        else {
            EXPECT_EQ(0, resolved->getObject("a")->count(k.first));
        }
    }
    if (existed == 0) {
        FAIL() << "None of the env vars we tried to use for testing were set";
    }
}

TEST_F(ConfigSubstitutionTest, throwWhenEnvNotFound) {
    auto obj = parseObject("{ a : ${NOT_HERE} }");
    EXPECT_THROW(resolve(obj), ConfigExceptionUnresolvedSubstitution);
}

TEST_F(ConfigSubstitutionTest, optionalOverrideNotProvided) {
    auto obj = parseObject("{ a: 42, a : ${?NOT_HERE} }");
    auto resolved = resolve(obj);
    EXPECT_EQ(42, resolved->getInt("a"));
}

TEST_F(ConfigSubstitutionTest, optionalOverrideProvided) {
    auto obj = parseObject("{ HERE : 43, a: 42, a : ${?HERE} }");
    auto resolved = resolve(obj);
    EXPECT_EQ(43, resolved->getInt("a"));
}

TEST_F(ConfigSubstitutionTest, optionalOverrideOfObjectNotProvided) {
    auto obj = parseObject("{ a: { b : 42 }, a : ${?NOT_HERE} }");
    auto resolved = resolve(obj);
    EXPECT_EQ(42, resolved->getInt("a.b"));
}

TEST_F(ConfigSubstitutionTest, optionalOverrideOfObjectProvided) {
    auto obj = parseObject("{ HERE : 43, a: { b : 42 }, a : ${?HERE} }");
    auto resolved = resolve(obj);
    EXPECT_EQ(43, resolved->getInt("a"));
    EXPECT_FALSE(resolved->hasPath("a.b"));
}

TEST_F(ConfigSubstitutionTest, optionalVanishesFromArray) {
    auto obj = parseObject("{ a : [ 1, 2, 3, ${?NOT_HERE} ] }");
    auto resolved = resolve(obj);
    EXPECT_TRUE(VectorInt({1, 2, 3}) == resolved->getIntList("a"));
}

TEST_F(ConfigSubstitutionTest, optionalUsedInArray) {
    auto obj = parseObject("{ HERE: 4, a : [ 1, 2, 3, ${?HERE} ] }");
    auto resolved = resolve(obj);
    EXPECT_TRUE(VectorInt({1, 2, 3, 4}) == resolved->getIntList("a"));
}

TEST_F(ConfigSubstitutionTest, substSelfReference) {
    auto obj = parseObject("a=1, a=${a}");
    auto resolved = resolve(obj);
    EXPECT_EQ(1, resolved->getInt("a"));
}

TEST_F(ConfigSubstitutionTest, substSelfReferenceUndefined) {
    try {
        auto obj = parseObject("a=${a}");
        resolve(obj);
        FAIL() << "expected: ConfigExceptionUnresolvedSubstitution";
    }
    catch (ConfigExceptionUnresolvedSubstitution& e) {
        EXPECT_TRUE(boost::contains(e.what(), "cycle"));
    }
}

TEST_F(ConfigSubstitutionTest, substSelfReferenceOptional) {
    auto obj = parseObject("a=${?a}");
    auto resolved = resolve(obj);
    EXPECT_EQ(0, resolved->root()->size()) << "optional self reference disappears";
}

TEST_F(ConfigSubstitutionTest, substSelfReferenceAlongPath) {
    auto obj = parseObject("a.b=1, a.b=${a.b}");
    auto resolved = resolve(obj);
    EXPECT_EQ(1, resolved->getInt("a.b"));
}

TEST_F(ConfigSubstitutionTest, substSelfReferenceAlongLongerPath) {
    auto obj = parseObject("a.b.c=1, a.b.c=${a.b.c}");
    auto resolved = resolve(obj);
    EXPECT_EQ(1, resolved->getInt("a.b.c"));
}

TEST_F(ConfigSubstitutionTest, substSelfReferenceAlongPathMoreComplex) {
    // this is an example from the spec
    auto obj = parseObject("foo : { a : { c : 1 } }\n   foo : ${foo.a}\n    foo : { a : 2 }");
    auto resolved = resolve(obj);
    EXPECT_EQ(1, resolved->getInt("foo.c"));
    EXPECT_EQ(2, resolved->getInt("foo.a"));
}

TEST_F(ConfigSubstitutionTest, substSelfReferenceIndirect) {
    try {
        auto obj = parseObject("b=${a}, a=1, a=${b}");
        resolve(obj);
        FAIL() << "expected: ConfigExceptionUnresolvedSubstitution";
    }
    catch (ConfigExceptionUnresolvedSubstitution& e) {
        EXPECT_TRUE(boost::contains(e.what(), "cycle"));
    }
}

TEST_F(ConfigSubstitutionTest, substSelfReferenceDoubleIndirect) {
    try {
        auto obj = parseObject("b=${c}, c=${a}, a=1, a=${b}");
        resolve(obj);
        FAIL() << "expected: ConfigExceptionUnresolvedSubstitution";
    }
    catch (ConfigExceptionUnresolvedSubstitution& e) {
        EXPECT_TRUE(boost::contains(e.what(), "cycle"));
    }
}

TEST_F(ConfigSubstitutionTest, substSelfReferenceIndirectStackCycle) {
    // this situation is undefined, depends on
    // whether we resolve a or b first.
    auto obj = parseObject("a=1, b={c=5}, b=${a}, a=${b}");
    auto resolved = resolve(obj);
    auto option1 = parseObject(" b={c=5}, a={c=5} ")->toConfig();
    auto option2 = parseObject(" b=1, a=1 ")->toConfig();
    EXPECT_TRUE(std::dynamic_pointer_cast<ConfigBase>(resolved)->equals(std::dynamic_pointer_cast<ConfigBase>(option1)) ||
                std::dynamic_pointer_cast<ConfigBase>(resolved)->equals(std::dynamic_pointer_cast<ConfigBase>(option2)));
}

TEST_F(ConfigSubstitutionTest, substSelfReferenceObject) {
    auto obj = parseObject("a={b=5}, a=${a}");
    auto resolved = resolve(obj);
    EXPECT_EQ(5, resolved->getInt("a.b"));
}

TEST_F(ConfigSubstitutionTest, substSelfReferenceObjectAlongPath) {
    auto obj = parseObject("a.b={c=5}, a.b=${a.b}");
    auto resolved = resolve(obj);
    EXPECT_EQ(5, resolved->getInt("a.b.c"));
}

TEST_F(ConfigSubstitutionTest, substSelfReferenceInConcat) {
    auto obj = parseObject("a=1, a=${a}foo");
    auto resolved = resolve(obj);
    EXPECT_EQ("1foo", resolved->getString("a"));
}

TEST_F(ConfigSubstitutionTest, substSelfReferenceIndirectInConcat) {
    // this situation is undefined, depends on
    // whether we resolve a or b first. If b first
    // then there's an error because ${a} is undefined.
    // if a first then b=1foo and a=1foo.
    try {
        auto obj = parseObject("a=1, b=${a}foo, a=${b}");
        auto resolved = resolve(obj);
        auto option1 = parseObject("a:1foo,b:1foo")->toConfig();
        EXPECT_TRUE(std::dynamic_pointer_cast<ConfigBase>(resolved)->equals(std::dynamic_pointer_cast<ConfigBase>(option1)));
    }
    catch (ConfigExceptionUnresolvedSubstitution& e) {
        EXPECT_TRUE(boost::contains(e.what(), "cycle"));
    }
}

TEST_F(ConfigSubstitutionTest, substOptionalSelfReferenceInConcat) {
    auto obj = parseObject("a=${?a}foo");
    auto resolved = resolve(obj);
    EXPECT_EQ("foo", resolved->getString("a"));
}

TEST_F(ConfigSubstitutionTest, substOptionalIndirectSelfReferenceInConcat) {
    auto obj = parseObject("b=${a},a=${?b}foo");
    auto resolved = resolve(obj);
    EXPECT_EQ("foo", resolved->getString("a"));
}

TEST_F(ConfigSubstitutionTest, substTwoOptionalSelfReferencesInConcat) {
    auto obj = parseObject("a=${?a}foo${?a}");
    auto resolved = resolve(obj);
    EXPECT_EQ("foo", resolved->getString("a"));
}

TEST_F(ConfigSubstitutionTest, substTwoOptionalSelfReferencesInConcatWithPriorValue) {
    auto obj = parseObject("a=1,a=${?a}foo${?a}");
    auto resolved = resolve(obj);
    EXPECT_EQ("1foo1", resolved->getString("a"));
}

TEST_F(ConfigSubstitutionTest, substSelfReferenceMiddleOfStack) {
    auto obj = parseObject("a=1, a=${a}, a=2");
    auto resolved = resolve(obj);
    EXPECT_EQ(2, resolved->getInt("a"));
}

TEST_F(ConfigSubstitutionTest, substSelfReferenceObjectMiddleOfStack) {
    auto obj = parseObject("a={b=5}, a=${a}, a={c=6}");
    auto resolved = resolve(obj);
    EXPECT_EQ(5, resolved->getInt("a.b"));
    EXPECT_EQ(6, resolved->getInt("a.c"));
}

TEST_F(ConfigSubstitutionTest, substOptionalSelfReferenceMiddleOfStack) {
    auto obj = parseObject("a=1, a=${?a}, a=2");
    auto resolved = resolve(obj);
    // the substitution would be 1, but then 2 overrides
    EXPECT_EQ(2, resolved->getInt("a"));
}

TEST_F(ConfigSubstitutionTest, substSelfReferenceBottomOfStack) {
    // self-reference should just be ignored since it's
    // overridden
    auto obj = parseObject("a=${a}, a=1, a=2");
    auto resolved = resolve(obj);
    EXPECT_EQ(2, resolved->getInt("a"));
}

TEST_F(ConfigSubstitutionTest, substOptionalSelfReferenceBottomOfStack) {
    auto obj = parseObject("a=${?a}, a=1, a=2");
    auto resolved = resolve(obj);
    EXPECT_EQ(2, resolved->getInt("a"));
}

TEST_F(ConfigSubstitutionTest, substSelfReferenceTopOfStack) {
    auto obj = parseObject("a=1, a=2, a=${a}");
    auto resolved = resolve(obj);
    EXPECT_EQ(2, resolved->getInt("a"));
}

TEST_F(ConfigSubstitutionTest, substOptionalSelfReferenceTopOfStack) {
    auto obj = parseObject("a=1, a=2, a=${?a}");
    auto resolved = resolve(obj);
    EXPECT_EQ(2, resolved->getInt("a"));
}

TEST_F(ConfigSubstitutionTest, substSelfReferenceAlongAPath) {
    // ${a} in the middle of the stack means "${a} in the stack
    // below us" and so ${a.b} means b inside the "${a} below us"
    // not b inside the final "${a}"
    auto obj = parseObject("a={b={c=5}}, a=${a.b}, a={b=2}");
    auto resolved = resolve(obj);
    EXPECT_EQ(5, resolved->getInt("a.c"));
}

TEST_F(ConfigSubstitutionTest, substSelfReferenceAlongAPathInsideObject) {
    // if the ${a.b} is _inside_ a field value instead of
    // _being_ the field value, it does not look backward.
    auto obj = parseObject("a={b={c=5}}, a={ x : ${a.b} }, a={b=2}");
    auto resolved = resolve(obj);
    EXPECT_EQ(2, resolved->getInt("a.x"));
}

TEST_F(ConfigSubstitutionTest, substInChildFieldNotASelfReference1) {
    // here, ${bar.foo} is not a self reference because
    // it's the value of a child field of bar, not bar
    // itself; so we use bar's current value, rather than
    // looking back in the merge stack
    auto obj = parseObject("\n"
                           "bar : { foo : 42,\n"
                           "        baz : ${bar.foo}\n"
                           "}\n"
                           "    ");
    auto resolved = resolve(obj);
    EXPECT_EQ(42, resolved->getInt("bar.baz"));
    EXPECT_EQ(42, resolved->getInt("bar.foo"));
}

TEST_F(ConfigSubstitutionTest, substInChildFieldNotASelfReference2) {
    // checking that having bar.foo later in the stack
    // doesn't break the behavior
    auto obj = parseObject("\n"
                           "bar : { foo : 42,\n"
                           "        baz : ${bar.foo}\n"
                           "}\n"
                           "bar : { foo : 43 }\n"
                           "    ");
    auto resolved = resolve(obj);
    EXPECT_EQ(43, resolved->getInt("bar.baz"));
    EXPECT_EQ(43, resolved->getInt("bar.foo"));
}

TEST_F(ConfigSubstitutionTest, substInChildFieldNotASelfReference3) {
    // checking that having bar.foo earlier in the merge
    // stack doesn't break the behavior.
    auto obj = parseObject("\n"
                           "bar : { foo : 43 }\n"
                           "bar : { foo : 42,\n"
                           "        baz : ${bar.foo}\n"
                           "}\n"
                           "    ");
    auto resolved = resolve(obj);
    EXPECT_EQ(42, resolved->getInt("bar.baz"));
    EXPECT_EQ(42, resolved->getInt("bar.foo"));
}

TEST_F(ConfigSubstitutionTest, substInChildFieldNotASelfReference4) {
    // checking that having bar set to non-object earlier
    // doesn't break the behavior.
    auto obj = parseObject("\n"
                           "bar : 101\n"
                           "bar : { foo : 42,\n"
                           "        baz : ${bar.foo}\n"
                           "}\n"
                           "    ");
    auto resolved = resolve(obj);
    EXPECT_EQ(42, resolved->getInt("bar.baz"));
    EXPECT_EQ(42, resolved->getInt("bar.foo"));
}

TEST_F(ConfigSubstitutionTest, substInChildFieldNotASelfReference5) {
    // checking that having bar set to unresolved array earlier
    // doesn't break the behavior.
    auto obj = parseObject("\n"
                           "x : 0\n"
                           "bar : [ ${x}, 1, 2, 3 ]\n"
                           "bar : { foo : 42,\n"
                           "        baz : ${bar.foo}\n"
                           "}\n"
                           "    ");
    auto resolved = resolve(obj);
    EXPECT_EQ(42, resolved->getInt("bar.baz"));
    EXPECT_EQ(42, resolved->getInt("bar.foo"));
}

TEST_F(ConfigSubstitutionTest, mutuallyReferringNotASelfReference) {
    // checking that having bar set to unresolved array earlier
    // doesn't break the behavior.
    auto obj = parseObject("\n"
                           "// bar.a should end up as 4\n"
                           "bar : { a : ${foo.d}, b : 1 }\n"
                           "bar.b = 3\n"
                           "// foo.c should end up as 3\n"
                           "foo : { c : ${bar.b}, d : 2 }\n"
                           "foo.d = 4\n"
                           "    ");
    auto resolved = resolve(obj);
    EXPECT_EQ(4, resolved->getInt("bar.a"));
    EXPECT_EQ(3, resolved->getInt("foo.c"));
}

TEST_F(ConfigSubstitutionTest, substSelfReferenceMultipleTimes) {
    auto obj = parseObject("a=1,a=${a},a=${a},a=${a}");
    auto resolved = resolve(obj);
    EXPECT_EQ(1, resolved->getInt("a"));
}

TEST_F(ConfigSubstitutionTest, substSelfReferenceInConcatMultipleTimes) {
    auto obj = parseObject("a=1,a=${a}x,a=${a}y,a=${a}z");
    auto resolved = resolve(obj);
    EXPECT_EQ("1xyz", resolved->getString("a"));
}

TEST_F(ConfigSubstitutionTest, substSelfReferenceInArray) {
    // never "look back" from "inside" an array
    try {
        auto obj = parseObject("a=1,a=[${a}, 2]");
        resolve(obj);
        FAIL() << "expected: ConfigExceptionUnresolvedSubstitution";
    }
    catch (ConfigExceptionUnresolvedSubstitution& e) {
        EXPECT_TRUE(boost::contains(e.what(), "cycle"));
        EXPECT_TRUE(boost::contains(e.what(), "${a}"));
    }
}

TEST_F(ConfigSubstitutionTest, substSelfReferenceInObject) {
    // never "look back" from "inside" an object
    try {
        auto obj = parseObject("a=1,a={ x : ${a} }");
        resolve(obj);
        FAIL() << "expected: ConfigExceptionUnresolvedSubstitution";
    }
    catch (ConfigExceptionUnresolvedSubstitution& e) {
        EXPECT_TRUE(boost::contains(e.what(), "cycle"));
        EXPECT_TRUE(boost::contains(e.what(), "${a}"));
    }
}

TEST_F(ConfigSubstitutionTest, selfReferentialObjectNotAffectedByOverriding) {
    // this is testing that we can still refer to another
    // field in the same object, even though we are overriding
    // an earlier object.
    auto obj = parseObject("a={ x : 42, y : ${a.x} }");
    auto resolved = resolve(obj);
    checkEquals(parseObject("{ x : 42, y : 42 }"), std::dynamic_pointer_cast<AbstractConfigObject>(resolved->getConfig("a")->root()));

    // this is expected because if adding "a=1" here affects the outcome,
    // it would be flat-out bizarre.
    auto obj2 = parseObject("a=1, a={ x : 42, y : ${a.x} }");
    auto resolved2 = resolve(obj);
    checkEquals(parseObject("{ x : 42, y : 42 }"), std::dynamic_pointer_cast<AbstractConfigObject>(resolved2->getConfig("a")->root()));
}
