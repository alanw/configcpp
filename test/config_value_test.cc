/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "test_fixture.h"
#include "configcpp/detail/abstract_config_value.h"
#include "configcpp/detail/simple_config_origin.h"
#include "configcpp/detail/simple_config_object.h"
#include "configcpp/detail/simple_config_list.h"
#include "configcpp/detail/config_reference.h"
#include "configcpp/detail/config_concatenation.h"
#include "configcpp/detail/config_delayed_merge.h"
#include "configcpp/detail/config_delayed_merge_object.h"
#include "configcpp/detail/config_number.h"
#include "configcpp/detail/resolve_status.h"
#include "configcpp/config.h"
#include "configcpp/config_value_type.h"

using namespace config;

class ConfigValueTest : public TestFixture {
};

TEST_F(ConfigValueTest, configOriginEquality) {
    auto a = SimpleConfigOrigin::newSimple("foo");
    auto sameAsA = SimpleConfigOrigin::newSimple("foo");
    auto b = SimpleConfigOrigin::newSimple("bar");

    checkEqualObjects(a, a);
    checkEqualObjects(a, sameAsA);
    checkNotEqualObjects(a, b);
}

TEST_F(ConfigValueTest, configIntEquality) {
    auto a = intValue(42);
    auto sameAsA = intValue(42);
    auto b = intValue(43);

    checkEqualObjects(a, a);
    checkEqualObjects(a, sameAsA);
    checkNotEqualObjects(a, b);
}

TEST_F(ConfigValueTest, configInt64Equality) {
    auto a = int64Value(std::numeric_limits<int32_t>::max() + 42LL);
    auto sameAsA = int64Value(std::numeric_limits<int32_t>::max() + 42LL);
    auto b = int64Value(std::numeric_limits<int32_t>::max() + 43LL);

    checkEqualObjects(a, a);
    checkEqualObjects(a, sameAsA);
    checkNotEqualObjects(a, b);
}

TEST_F(ConfigValueTest, configIntAndInt64Equality) {
    auto int64Val = int64Value(42LL);
    auto intValue = int64Value(42);
    auto int64ValueB = int64Value(43LL);
    auto intValueB = int64Value(43);

    checkEqualObjects(intValue, int64Val);
    checkEqualObjects(intValueB, int64ValueB);
    checkNotEqualObjects(intValue, int64ValueB);
    checkNotEqualObjects(intValueB, int64Val);
}

TEST_F(ConfigValueTest, configDoubleEquality) {
    auto a = doubleValue(3.14);
    auto sameAsA = doubleValue(3.14);
    auto b = doubleValue(4.14);

    checkEqualObjects(a, a);
    checkEqualObjects(a, sameAsA);
    checkNotEqualObjects(a, b);
}

TEST_F(ConfigValueTest, configIntAndDoubleEquality) {
    auto doubleVal = doubleValue(3.0);
    auto intValue = int64Value(3);
    auto doubleValueB = doubleValue(4.0);
    auto intValueB = doubleValue(4);

    checkEqualObjects(intValue, doubleVal);
    checkEqualObjects(intValueB, doubleValueB);
    checkNotEqualObjects(intValue, doubleValueB);
    checkNotEqualObjects(intValueB, doubleVal);
}

TEST_F(ConfigValueTest, configObjectEquality) {
    auto aMap = MapAbstractConfigValue({{"a", intValue(1)}, {"b", intValue(2)}, {"c", intValue(3)}});
    auto sameAsAMap = MapAbstractConfigValue({{"a", intValue(1)}, {"b", intValue(2)}, {"c", intValue(3)}});
    auto bMap = MapAbstractConfigValue({{"a", intValue(3)}, {"b", intValue(4)}, {"c", intValue(5)}});
    // different keys is a different case in the equals implementation
    auto cMap = MapAbstractConfigValue({{"x", intValue(3)}, {"y", intValue(4)}, {"z", intValue(5)}});
    auto a = SimpleConfigObject::make_instance(fakeOrigin(), aMap);
    auto sameAsA = SimpleConfigObject::make_instance(fakeOrigin(), sameAsAMap);
    auto b = SimpleConfigObject::make_instance(fakeOrigin(), bMap);
    auto c = SimpleConfigObject::make_instance(fakeOrigin(), cMap);

    checkEqualObjects(a, a);
    checkEqualObjects(a, sameAsA);
    checkEqualObjects(b, b);
    checkEqualObjects(c, c);
    checkNotEqualObjects(a, b);
    checkNotEqualObjects(a, c);
    checkNotEqualObjects(b, c);

    // the config for an equal object is also equal
    auto config = a->toConfig();
    checkEqualObjects(std::dynamic_pointer_cast<ConfigBase>(config), std::dynamic_pointer_cast<ConfigBase>(config));
    checkEqualObjects(std::dynamic_pointer_cast<ConfigBase>(config), std::dynamic_pointer_cast<ConfigBase>(sameAsA->toConfig()));
    checkEqualObjects(std::dynamic_pointer_cast<ConfigBase>(a->toConfig()), std::dynamic_pointer_cast<ConfigBase>(config));
    checkNotEqualObjects(std::dynamic_pointer_cast<ConfigBase>(config), std::dynamic_pointer_cast<ConfigBase>(b->toConfig()));
    checkNotEqualObjects(std::dynamic_pointer_cast<ConfigBase>(config), std::dynamic_pointer_cast<ConfigBase>(c->toConfig()));

    // configs are not equal to objects
    checkNotEqualObjects(std::dynamic_pointer_cast<ConfigBase>(a), std::dynamic_pointer_cast<ConfigBase>(a->toConfig()));
    checkNotEqualObjects(std::dynamic_pointer_cast<ConfigBase>(b), std::dynamic_pointer_cast<ConfigBase>(b->toConfig()));
}

TEST_F(ConfigValueTest, configListEquality) {
    auto aScalaSeq = VectorAbstractConfigValue({intValue(1), intValue(2), intValue(3)});
    auto aList = SimpleConfigList::make_instance(fakeOrigin(), aScalaSeq);
    auto sameAsAList = SimpleConfigList::make_instance(fakeOrigin(), aScalaSeq);
    auto bScalaSeq = VectorAbstractConfigValue({intValue(4), intValue(5), intValue(6)});
    auto bList = SimpleConfigList::make_instance(fakeOrigin(), bScalaSeq);

    checkEqualObjects(aList, aList);
    checkEqualObjects(aList, sameAsAList);
    checkNotEqualObjects(aList, bList);
}

TEST_F(ConfigValueTest, configReferenceEquality) {
    auto a = subst("foo");
    auto sameAsA = subst("foo");
    auto b = subst("bar");
    auto c = subst("foo", true);

    EXPECT_TRUE(instanceof<ConfigReference>(a)) << "Wrong type";
    EXPECT_TRUE(instanceof<ConfigReference>(b)) << "Wrong type";
    EXPECT_TRUE(instanceof<ConfigReference>(c)) << "Wrong type";

    checkEqualObjects(std::dynamic_pointer_cast<ConfigBase>(a), std::dynamic_pointer_cast<ConfigBase>(a));
    checkEqualObjects(std::dynamic_pointer_cast<ConfigBase>(a), std::dynamic_pointer_cast<ConfigBase>(sameAsA));
    checkNotEqualObjects(std::dynamic_pointer_cast<ConfigBase>(a), std::dynamic_pointer_cast<ConfigBase>(b));
    checkNotEqualObjects(std::dynamic_pointer_cast<ConfigBase>(a), std::dynamic_pointer_cast<ConfigBase>(c));
}

TEST_F(ConfigValueTest, configConcatenationEquality) {
    auto a = substInString("foo");
    auto sameAsA = substInString("foo");
    auto b = substInString("bar");
    auto c = substInString("foo", true);

    EXPECT_TRUE(instanceof<ConfigConcatenation>(a)) << "Wrong type";
    EXPECT_TRUE(instanceof<ConfigConcatenation>(b)) << "Wrong type";
    EXPECT_TRUE(instanceof<ConfigConcatenation>(c)) << "Wrong type";

    checkEqualObjects(std::dynamic_pointer_cast<ConfigBase>(a), std::dynamic_pointer_cast<ConfigBase>(a));
    checkEqualObjects(std::dynamic_pointer_cast<ConfigBase>(a), std::dynamic_pointer_cast<ConfigBase>(sameAsA));
    checkNotEqualObjects(std::dynamic_pointer_cast<ConfigBase>(a), std::dynamic_pointer_cast<ConfigBase>(b));
    checkNotEqualObjects(std::dynamic_pointer_cast<ConfigBase>(a), std::dynamic_pointer_cast<ConfigBase>(c));
}

TEST_F(ConfigValueTest, configDelayedMergeEquality) {
    auto s1 = subst("foo");
    auto s2 = subst("bar");
    auto a = ConfigDelayedMerge::make_instance(fakeOrigin(), VectorAbstractConfigValue({s1, s2}));
    auto sameAsA = ConfigDelayedMerge::make_instance(fakeOrigin(), VectorAbstractConfigValue({s1, s2}));
    auto b = ConfigDelayedMerge::make_instance(fakeOrigin(), VectorAbstractConfigValue({s2, s1}));

    checkEqualObjects(std::dynamic_pointer_cast<ConfigBase>(a), std::dynamic_pointer_cast<ConfigBase>(a));
    checkEqualObjects(std::dynamic_pointer_cast<ConfigBase>(a), std::dynamic_pointer_cast<ConfigBase>(sameAsA));
    checkNotEqualObjects(std::dynamic_pointer_cast<ConfigBase>(a), std::dynamic_pointer_cast<ConfigBase>(b));
}

TEST_F(ConfigValueTest, configDelayedMergeObjectEquality) {
    auto empty = SimpleConfigObject::makeEmpty();
    auto s1 = subst("foo");
    auto s2 = subst("bar");
    auto a = ConfigDelayedMergeObject::make_instance(fakeOrigin(), VectorAbstractConfigValue({empty, s1, s2}));
    auto sameAsA = ConfigDelayedMergeObject::make_instance(fakeOrigin(), VectorAbstractConfigValue({empty, s1, s2}));
    auto b = ConfigDelayedMergeObject::make_instance(fakeOrigin(), VectorAbstractConfigValue({empty, s2, s1}));

    checkEqualObjects(std::dynamic_pointer_cast<ConfigBase>(a), std::dynamic_pointer_cast<ConfigBase>(a));
    checkEqualObjects(std::dynamic_pointer_cast<ConfigBase>(a), std::dynamic_pointer_cast<ConfigBase>(sameAsA));
    checkNotEqualObjects(std::dynamic_pointer_cast<ConfigBase>(a), std::dynamic_pointer_cast<ConfigBase>(b));
}

TEST_F(ConfigValueTest, valuesToString) {
    // just check that these don't throw, the exact output
    // isn't super important since it's just for debugging
    intValue(10)->toString();
    int64Value(11)->toString();
    doubleValue(3.14)->toString();
    stringValue("hi")->toString();
    nullValue()->toString();
    boolValue(true)->toString();
    auto emptyObj = SimpleConfigObject::makeEmpty();
    emptyObj->toString();
    SimpleConfigList::make_instance(fakeOrigin(), VectorAbstractConfigValue())->toString();
    subst("a")->toString();
    substInString("b")->toString();
    auto dm = ConfigDelayedMerge::make_instance(fakeOrigin(), VectorAbstractConfigValue({subst("a"), subst("b")}));
    dm->toString();
    auto dmo = ConfigDelayedMergeObject::make_instance(fakeOrigin(), VectorAbstractConfigValue({emptyObj, subst("a"), subst("b")}));
    dmo->toString();

    fakeOrigin()->toString();
}

TEST_F(ConfigValueTest, configObjectUnwraps) {
    auto m = SimpleConfigObject::make_instance(fakeOrigin(), MapAbstractConfigValue({{"a", intValue(1)}, {"b", intValue(2)}, {"c", intValue(3)}}));
    ConfigVariant test = m->unwrapped<MapVariant>();
    ConfigVariant expected = MapVariant({{"a", 1}, {"b", 2}, {"c", 3}});
    EXPECT_TRUE(boost::apply_visitor(VariantEquals(expected), test));
}

TEST_F(ConfigValueTest, configObjectImplementsMap) {
    auto m = SimpleConfigObject::make_instance(fakeOrigin(), MapAbstractConfigValue({{"a", intValue(1)}, {"b", intValue(2)}, {"c", intValue(3)}}));

    checkEquals(intValue(1), std::dynamic_pointer_cast<ConfigBase>(m->get("a")));
    checkEquals(intValue(2), std::dynamic_pointer_cast<ConfigBase>(m->get("b")));
    checkEquals(intValue(3), std::dynamic_pointer_cast<ConfigBase>(m->get("c")));
    EXPECT_TRUE(m->find("a") != m->end());
    EXPECT_TRUE(m->find("b") != m->end());
    EXPECT_TRUE(m->find("c") != m->end());

    EXPECT_FALSE(m->get("d"));
    EXPECT_TRUE(m->find("d") == m->end());

    EXPECT_TRUE(m->count("a") > 0);
    EXPECT_FALSE(m->count("z") > 0);

    EXPECT_FALSE(m->empty());
    EXPECT_EQ(3, m->size());

    EXPECT_THROW(m->clear(), ConfigExceptionUnsupportedOperation);
    EXPECT_THROW(m->insert(std::make_pair("hello", intValue(42))), ConfigExceptionUnsupportedOperation);
    EXPECT_THROW(m->erase("hello"), ConfigExceptionUnsupportedOperation);
}

TEST_F(ConfigValueTest, configListImplementsList) {
    auto seq = VectorAbstractConfigValue({stringValue("a"), stringValue("b"), stringValue("c")});
    auto l = SimpleConfigList::make_instance(fakeOrigin(), seq);

    checkEquals(seq[0], std::dynamic_pointer_cast<ConfigBase>(l->at(0)));
    checkEquals(seq[1], std::dynamic_pointer_cast<ConfigBase>(l->at(1)));
    checkEquals(seq[2], std::dynamic_pointer_cast<ConfigBase>(l->at(2)));

    EXPECT_FALSE(l->empty());
    EXPECT_EQ(3, l->size());

    checkEquals(seq[0], std::dynamic_pointer_cast<ConfigBase>(l->front()));
    checkEquals(seq[2], std::dynamic_pointer_cast<ConfigBase>(l->back()));

    EXPECT_THROW(l->clear(), ConfigExceptionUnsupportedOperation);
    EXPECT_THROW(l->pop_back(), ConfigExceptionUnsupportedOperation);
    EXPECT_THROW(l->resize(0), ConfigExceptionUnsupportedOperation);
    EXPECT_THROW(l->erase(l->begin()), ConfigExceptionUnsupportedOperation);
    EXPECT_THROW(l->insert(l->begin(), stringValue("d")), ConfigExceptionUnsupportedOperation);
}

TEST_F(ConfigValueTest, notResolvedThrown) {
    // ConfigSubstitution
    EXPECT_THROW(subst("foo")->valueType(), ConfigExceptionNotResolved);
    EXPECT_THROW(subst("foo")->unwrapped(), ConfigExceptionNotResolved);

    // ConfigDelayedMerge
    auto dm = ConfigDelayedMerge::make_instance(fakeOrigin(), VectorAbstractConfigValue({subst("a"), subst("b")}));
    EXPECT_THROW(dm->valueType(), ConfigExceptionNotResolved);
    EXPECT_THROW(dm->unwrapped(), ConfigExceptionNotResolved);

    // ConfigDelayedMergeObject
    auto emptyObj = SimpleConfigObject::makeEmpty();
    auto dmo = ConfigDelayedMergeObject::make_instance(fakeOrigin(), VectorAbstractConfigValue({emptyObj, subst("a"), subst("b")}));
    EXPECT_TRUE(ConfigValueType::OBJECT == dmo->valueType());
    EXPECT_THROW(dmo->unwrapped(), ConfigExceptionNotResolved);
    EXPECT_THROW(dmo->get("foo"), ConfigExceptionNotResolved);
    EXPECT_THROW(dmo->find("foo"), ConfigExceptionNotResolved);
    EXPECT_THROW(dmo->count("foo"), ConfigExceptionNotResolved);
    EXPECT_THROW(dmo->empty(), ConfigExceptionNotResolved);
    EXPECT_THROW(dmo->size(), ConfigExceptionNotResolved);
    EXPECT_THROW(dmo->toConfig()->getInt("foo"), ConfigExceptionNotResolved);
}

TEST_F(ConfigValueTest, roundTripNumbersThroughString) {
    // formats rounded off with E notation
    std::string a = "132454454354353245.3254652656454808909932874873298473298472";
    // formats as 100000.0
    std::string b = "1e6";
    // formats as 5.0E-5
    std::string c = "0.00005";
    // formats as 1E100 (capital E)
    std::string d = "1e100";

    auto obj = parseConfig("{ a : " + a + ", b : " + b + ", c : " + c + ", d : " + d + "}");
    EXPECT_TRUE(VectorString({a, b, c, d}) == VectorString({obj->getString("a"), obj->getString("b"), obj->getString("c"), obj->getString("d")}));

    // make sure it still works if we're doing concatenation
    auto obj2 = parseConfig("{ a : xx " + a + " yy, b : xx " + b + " yy, c : xx " + c + " yy, d : xx " + d + " yy}");
    EXPECT_TRUE(VectorString({"xx " + a + " yy", "xx " + b + " yy", "xx " + c + " yy", "xx " + d + " yy"}) == VectorString({obj2->getString("a"), obj2->getString("b"), obj2->getString("c"), obj2->getString("d")}));
}

TEST_F(ConfigValueTest, mergeOriginsWorks) {
    // simplest case
    EXPECT_EQ(
        "merge of a,b",
        AbstractConfigObject::mergeOrigins(
            VectorAbstractConfigValue({
                SimpleConfigObject::make_instance(
                    SimpleConfigOrigin::newSimple("a"),
                    MapAbstractConfigValue({{"hello", intValue(37)}})
                ),
                SimpleConfigObject::make_instance(
                    SimpleConfigOrigin::newSimple("b"),
                    MapAbstractConfigValue({{"hello", intValue(37)}})
                )
            })
        )->description()
    );

    // combine duplicate "merge of"
    EXPECT_EQ(
        "merge of a,x,y",
        AbstractConfigObject::mergeOrigins(
            VectorAbstractConfigValue({
                SimpleConfigObject::make_instance(
                    SimpleConfigOrigin::newSimple("a"),
                    MapAbstractConfigValue({{"hello", intValue(37)}})
                ),
                SimpleConfigObject::make_instance(
                    SimpleConfigOrigin::newSimple("merge of x,y"),
                    MapAbstractConfigValue({{"hello", intValue(37)}})
                )
            })
        )->description()
    );

    EXPECT_EQ(
        "merge of a,b,x,y",
        AbstractConfigObject::mergeOrigins(
            VectorAbstractConfigValue({
                SimpleConfigObject::make_instance(
                    SimpleConfigOrigin::newSimple("merge of a,b"),
                    MapAbstractConfigValue({{"hello", intValue(37)}})
                ),
                SimpleConfigObject::make_instance(
                    SimpleConfigOrigin::newSimple("merge of x,y"),
                    MapAbstractConfigValue({{"hello", intValue(37)}})
                )
            })
        )->description()
    );

    // ignore empty objects
    EXPECT_EQ(
        "a",
        AbstractConfigObject::mergeOrigins(
            VectorAbstractConfigValue({
                SimpleConfigObject::make_instance(
                    SimpleConfigOrigin::newSimple("foo"),
                    MapAbstractConfigValue()
                ),
                SimpleConfigObject::make_instance(
                    SimpleConfigOrigin::newSimple("a"),
                    MapAbstractConfigValue({{"hello", intValue(37)}})
                )
            })
        )->description()
    );

    // unless they are all empty, pick the first one
    EXPECT_EQ(
        "foo",
        AbstractConfigObject::mergeOrigins(
            VectorAbstractConfigValue({
                SimpleConfigObject::make_instance(
                    SimpleConfigOrigin::newSimple("foo"),
                    MapAbstractConfigValue()
                ),
                SimpleConfigObject::make_instance(
                    SimpleConfigOrigin::newSimple("a"),
                    MapAbstractConfigValue()
                )
            })
        )->description()
    );

    // merge just one
    EXPECT_EQ(
        "foo",
        AbstractConfigObject::mergeOrigins(
            VectorAbstractConfigValue({
                SimpleConfigObject::make_instance(
                    SimpleConfigOrigin::newSimple("foo"),
                    MapAbstractConfigValue({{"hello", intValue(37)}})
                )
            })
        )->description()
    );

    // merge three
    EXPECT_EQ(
        "merge of a,b,c",
        AbstractConfigObject::mergeOrigins(
            VectorAbstractConfigValue({
                SimpleConfigObject::make_instance(
                    SimpleConfigOrigin::newSimple("a"),
                    MapAbstractConfigValue({{"hello", intValue(37)}})
                ),
                SimpleConfigObject::make_instance(
                    SimpleConfigOrigin::newSimple("b"),
                    MapAbstractConfigValue({{"hello", intValue(37)}})
                ),
                SimpleConfigObject::make_instance(
                    SimpleConfigOrigin::newSimple("c"),
                    MapAbstractConfigValue({{"hello", intValue(37)}})
                )
            })
        )->description()
    );
}

TEST_F(ConfigValueTest, hasPathWorks) {
    auto empty = parseConfig("{}");

    EXPECT_FALSE(empty->hasPath("foo"));

    auto obj = parseConfig("a=null, b.c.d=11, foo=bar");

    // returns true for the non-null values
    EXPECT_TRUE(obj->hasPath("foo"));
    EXPECT_TRUE(obj->hasPath("b.c.d"));
    EXPECT_TRUE(obj->hasPath("b.c"));
    EXPECT_TRUE(obj->hasPath("b"));

    // hasPath() is false for null values but containsKey is true
    checkEquals(nullValue(), std::dynamic_pointer_cast<AbstractConfigValue>(obj->root()->get("a")));
    EXPECT_TRUE(obj->root()->count("a") > 0);
    EXPECT_FALSE(obj->hasPath("a"));

    // false for totally absent values
    EXPECT_FALSE(obj->root()->count("notinhere") > 0);
    EXPECT_FALSE(obj->hasPath("notinhere"));

    // throws proper exceptions
    EXPECT_THROW(empty->hasPath("a."), ConfigExceptionBadPath);
    EXPECT_THROW(empty->hasPath(".."), ConfigExceptionBadPath);
}

TEST_F(ConfigValueTest, newNumberWorks) {
    // the general idea is that the destination type should depend
    // only on the actual numeric value, not on the type of the source
    // value.
    EXPECT_EQ(3.14, ConfigNumber::newNumber(fakeOrigin(), 3.14, "")->unwrapped<double>());
    EXPECT_EQ(1, ConfigNumber::newNumber(fakeOrigin(), 1LL, "")->unwrapped<int32_t>());
    EXPECT_EQ(1, ConfigNumber::newNumber(fakeOrigin(), 1.0, "")->unwrapped<int32_t>());
    EXPECT_EQ(std::numeric_limits<int32_t>::max() + 1LL, ConfigNumber::newNumber(fakeOrigin(), std::numeric_limits<int32_t>::max() + 1LL, "")->unwrapped<int64_t>());
    EXPECT_EQ(std::numeric_limits<int32_t>::min() - 1LL, ConfigNumber::newNumber(fakeOrigin(), std::numeric_limits<int32_t>::min() - 1LL, "")->unwrapped<int64_t>());
    EXPECT_EQ(std::numeric_limits<int32_t>::max() + 1LL, ConfigNumber::newNumber(fakeOrigin(), std::numeric_limits<int32_t>::max() + 1.0, "")->unwrapped<int64_t>());
    EXPECT_EQ(std::numeric_limits<int32_t>::min() - 1LL, ConfigNumber::newNumber(fakeOrigin(), std::numeric_limits<int32_t>::min() - 1.0, "")->unwrapped<int64_t>());
}

TEST_F(ConfigValueTest, automaticBooleanConversions) {
    auto trues = parseObject("{ a=true, b=yes, c=on }")->toConfig();
    EXPECT_TRUE(trues->getBoolean("a"));
    EXPECT_TRUE(trues->getBoolean("b"));
    EXPECT_TRUE(trues->getBoolean("c"));

    auto falses = parseObject("{ a=false, b=no, c=off }")->toConfig();
    EXPECT_FALSE(falses->getBoolean("a"));
    EXPECT_FALSE(falses->getBoolean("b"));
    EXPECT_FALSE(falses->getBoolean("c"));
}

TEST_F(ConfigValueTest, configOriginFileAndLine) {
    auto hasFilename = SimpleConfigOrigin::newFile("foo");
    auto noFilename = SimpleConfigOrigin::newSimple("bar");
    auto filenameWithLine = hasFilename->setLineNumber(3);
    auto noFilenameWithLine = noFilename->setLineNumber(4);

    EXPECT_EQ("foo", hasFilename->filename());
    EXPECT_EQ("foo", filenameWithLine->filename());
    EXPECT_EQ("", noFilename->filename());
    EXPECT_EQ("", noFilenameWithLine->filename());

    EXPECT_EQ("foo", hasFilename->description());
    EXPECT_EQ("bar", noFilename->description());

    EXPECT_EQ(-1, hasFilename->lineNumber());
    EXPECT_EQ(-1, noFilename->lineNumber());

    EXPECT_EQ("foo: 3", filenameWithLine->description());
    EXPECT_EQ("bar: 4", noFilenameWithLine->description());

    EXPECT_EQ(3, filenameWithLine->lineNumber());
    EXPECT_EQ(4, noFilenameWithLine->lineNumber());
}

TEST_F(ConfigValueTest, withOnly) {
    auto obj = parseObject("{ a=1, b=2, c.d.y=3, e.f.g=4, c.d.z=5 }");
    checkEquals(parseObject("{ a=1 }"), std::dynamic_pointer_cast<ConfigBase>(obj->withOnlyKey("a")));
    checkEquals(parseObject("{ e.f.g=4 }"), std::dynamic_pointer_cast<ConfigBase>(obj->withOnlyKey("e")));
    checkEquals(parseObject("{ c.d.y=3, c.d.z=5 }"), std::dynamic_pointer_cast<ConfigBase>(obj->toConfig()->withOnlyPath("c.d")->root()));
    checkEquals(parseObject("{ c.d.z=5 }"), std::dynamic_pointer_cast<ConfigBase>(obj->toConfig()->withOnlyPath("c.d.z")->root()));
    checkEquals(parseObject("{ }"), std::dynamic_pointer_cast<ConfigBase>(obj->withOnlyKey("nope")));
    checkEquals(parseObject("{ }"), std::dynamic_pointer_cast<ConfigBase>(obj->toConfig()->withOnlyPath("q.w.e.r.t.y")->root()));
    checkEquals(parseObject("{ }"), std::dynamic_pointer_cast<ConfigBase>(obj->toConfig()->withOnlyPath("a.nonexistent")->root()));
    checkEquals(parseObject("{ }"), std::dynamic_pointer_cast<ConfigBase>(obj->toConfig()->withOnlyPath("c.d.z.nonexistent")->root()));
}

TEST_F(ConfigValueTest, withOnlyInvolvingUnresolved) {
    auto obj = parseObject("{ a = {}, a=${x}, b=${y}, b=${z}, x={asf:1}, y=2, z=3 }");
    checkEquals(parseObject("{ a={asf:1} }"), std::dynamic_pointer_cast<ConfigBase>(obj->toConfig()->resolve()->withOnlyPath("a.asf")->root()));

    EXPECT_THROW(obj->withOnlyKey("a")->toConfig()->resolve(), ConfigExceptionUnresolvedSubstitution);
    EXPECT_THROW(obj->withOnlyKey("b")->toConfig()->resolve(), ConfigExceptionUnresolvedSubstitution);

    EXPECT_TRUE(ResolveStatus::UNRESOLVED == obj->resolveStatus());
    EXPECT_TRUE(ResolveStatus::RESOLVED == std::dynamic_pointer_cast<AbstractConfigValue>(obj->withOnlyKey("z"))->resolveStatus());
}

TEST_F(ConfigValueTest, without) {
    auto obj = parseObject("{ a=1, b=2, c.d.y=3, e.f.g=4, c.d.z=5 }");
    checkEquals(parseObject("{ b=2, c.d.y=3, e.f.g=4, c.d.z=5 }"), std::dynamic_pointer_cast<ConfigBase>(obj->withoutKey("a")));
    checkEquals(parseObject("{ a=1, b=2, e.f.g=4 }"), std::dynamic_pointer_cast<ConfigBase>(obj->withoutKey("c")));
    checkEquals(parseObject("{ a=1, b=2, e.f.g=4, c={} }"), std::dynamic_pointer_cast<ConfigBase>(obj->toConfig()->withoutPath("c.d")->root()));
    checkEquals(parseObject("{ a=1, b=2, c.d.y=3, e.f.g=4 }"), std::dynamic_pointer_cast<ConfigBase>(obj->toConfig()->withoutPath("c.d.z")->root()));
    checkEquals(parseObject("{ a=1, b=2, c.d.y=3, e.f.g=4, c.d.z=5 }"), std::dynamic_pointer_cast<ConfigBase>(obj->withoutKey("nonexistent")));
    checkEquals(parseObject("{ a=1, b=2, c.d.y=3, e.f.g=4, c.d.z=5 }"), std::dynamic_pointer_cast<ConfigBase>(obj->toConfig()->withoutPath("q.w.e.r.t.y")->root()));
    checkEquals(parseObject("{ a=1, b=2, c.d.y=3, e.f.g=4, c.d.z=5 }"), std::dynamic_pointer_cast<ConfigBase>(obj->toConfig()->withoutPath("a.foo")->root()));
}

TEST_F(ConfigValueTest, withoutInvolvingUnresolved) {
    auto obj = parseObject("{ a = {}, a=${x}, b=${y}, b=${z}, x={asf:1}, y=2, z=3 }");
    checkEquals(parseObject("{ a={}, b=3, x={asf:1}, y=2, z=3 }"), std::dynamic_pointer_cast<ConfigBase>(obj->toConfig()->resolve()->withoutPath("a.asf")->root()));

    EXPECT_THROW(obj->withoutKey("x")->toConfig()->resolve(), ConfigExceptionUnresolvedSubstitution);
    EXPECT_THROW(obj->withoutKey("z")->toConfig()->resolve(), ConfigExceptionUnresolvedSubstitution);

    EXPECT_TRUE(ResolveStatus::UNRESOLVED == obj->resolveStatus());
    EXPECT_TRUE(ResolveStatus::UNRESOLVED == std::dynamic_pointer_cast<AbstractConfigValue>(obj->withoutKey("a"))->resolveStatus());
    EXPECT_TRUE(ResolveStatus::RESOLVED == std::dynamic_pointer_cast<AbstractConfigValue>(obj->withoutKey("a")->withoutKey("b"))->resolveStatus());
}

TEST_F(ConfigValueTest, atPathWorksOneElement) {
    auto v = ConfigValue::fromAnyRef(42);
    auto config = v->atPath("a");
    checkEquals(std::dynamic_pointer_cast<ConfigBase>(parseConfig("a=42")), std::dynamic_pointer_cast<ConfigBase>(config));
    checkSame(std::dynamic_pointer_cast<ConfigBase>(config->getValue("a")), std::dynamic_pointer_cast<ConfigBase>(v));
    EXPECT_TRUE(boost::contains(config->origin()->description(), "atPath"));
}

TEST_F(ConfigValueTest, atPathWorksTwoElements) {
    auto v = ConfigValue::fromAnyRef(42);
    auto config = v->atPath("a.b");
    checkEquals(std::dynamic_pointer_cast<ConfigBase>(parseConfig("a.b=42")), std::dynamic_pointer_cast<ConfigBase>(config));
    checkSame(std::dynamic_pointer_cast<ConfigBase>(config->getValue("a.b")), std::dynamic_pointer_cast<ConfigBase>(v));
    EXPECT_TRUE(boost::contains(config->origin()->description(), "atPath"));
}

TEST_F(ConfigValueTest, atPathWorksFourElements) {
    auto v = ConfigValue::fromAnyRef(42);
    auto config = v->atPath("a.b.c.d");
    checkEquals(std::dynamic_pointer_cast<ConfigBase>(parseConfig("a.b.c.d=42")), std::dynamic_pointer_cast<ConfigBase>(config));
    checkSame(std::dynamic_pointer_cast<ConfigBase>(config->getValue("a.b.c.d")), std::dynamic_pointer_cast<ConfigBase>(v));
    EXPECT_TRUE(boost::contains(config->origin()->description(), "atPath"));
}

TEST_F(ConfigValueTest, atKeyWorks) {
    auto v = ConfigValue::fromAnyRef(42);
    auto config = v->atKey("a");
    checkEquals(std::dynamic_pointer_cast<ConfigBase>(parseConfig("a=42")), std::dynamic_pointer_cast<ConfigBase>(config));
    checkSame(std::dynamic_pointer_cast<ConfigBase>(config->getValue("a")), std::dynamic_pointer_cast<ConfigBase>(v));
    EXPECT_TRUE(boost::contains(config->origin()->description(), "atKey"));
}

TEST_F(ConfigValueTest, withValueDepth1FromEmpty) {
    auto v = ConfigValue::fromAnyRef(42);
    auto config = Config::emptyConfig()->withValue("a", v);
    checkEquals(std::dynamic_pointer_cast<ConfigBase>(parseConfig("a=42")), std::dynamic_pointer_cast<ConfigBase>(config));
    checkSame(std::dynamic_pointer_cast<ConfigBase>(config->getValue("a")), std::dynamic_pointer_cast<ConfigBase>(v));
}

TEST_F(ConfigValueTest, withValueDepth2FromEmpty) {
    auto v = ConfigValue::fromAnyRef(42);
    auto config = Config::emptyConfig()->withValue("a.b", v);
    checkEquals(std::dynamic_pointer_cast<ConfigBase>(parseConfig("a.b=42")), std::dynamic_pointer_cast<ConfigBase>(config));
    checkSame(std::dynamic_pointer_cast<ConfigBase>(config->getValue("a.b")), std::dynamic_pointer_cast<ConfigBase>(v));
}

TEST_F(ConfigValueTest, withValueDepth3FromEmpty) {
    auto v = ConfigValue::fromAnyRef(42);
    auto config = Config::emptyConfig()->withValue("a.b.c", v);
    checkEquals(std::dynamic_pointer_cast<ConfigBase>(parseConfig("a.b.c=42")), std::dynamic_pointer_cast<ConfigBase>(config));
    checkSame(std::dynamic_pointer_cast<ConfigBase>(config->getValue("a.b.c")), std::dynamic_pointer_cast<ConfigBase>(v));
}

TEST_F(ConfigValueTest, withValueDepth1OverwritesExisting) {
    auto v = ConfigValue::fromAnyRef(47);
    auto old = v->atPath("a");
    auto config = old->withValue("a", ConfigValue::fromAnyRef(42));
    checkEquals(std::dynamic_pointer_cast<ConfigBase>(parseConfig("a=42")), std::dynamic_pointer_cast<ConfigBase>(config));
    EXPECT_EQ(42, config->getInt("a"));
}

TEST_F(ConfigValueTest, withValueDepth2OverwritesExisting) {
    auto v = ConfigValue::fromAnyRef(47);
    auto old = v->atPath("a.b");
    auto config = old->withValue("a.b", ConfigValue::fromAnyRef(42));
    checkEquals(std::dynamic_pointer_cast<ConfigBase>(parseConfig("a.b=42")), std::dynamic_pointer_cast<ConfigBase>(config));
    EXPECT_EQ(42, config->getInt("a.b"));
}

TEST_F(ConfigValueTest, withValueInsideExistingObject) {
    auto v = ConfigValue::fromAnyRef(47);
    auto old = v->atPath("a.c");
    auto config = old->withValue("a.b", ConfigValue::fromAnyRef(42));
    checkEquals(std::dynamic_pointer_cast<ConfigBase>(parseConfig("a.b=42,a.c=47")), std::dynamic_pointer_cast<ConfigBase>(config));
    EXPECT_EQ(42, config->getInt("a.b"));
    EXPECT_EQ(47, config->getInt("a.c"));
}

TEST_F(ConfigValueTest, withValueBuildComplexConfig) {
    auto v1 = ConfigValue::fromAnyRef(1);
    auto v2 = ConfigValue::fromAnyRef(2);
    auto v3 = ConfigValue::fromAnyRef(3);
    auto v4 = ConfigValue::fromAnyRef(4);
    auto config = Config::emptyConfig()
        ->withValue("a", v1)
        ->withValue("b.c", v2)
        ->withValue("b.d", v3)
        ->withValue("x.y.z", v4);
    checkEquals(std::dynamic_pointer_cast<ConfigBase>(parseConfig("a=1,b.c=2,b.d=3,x.y.z=4")), std::dynamic_pointer_cast<ConfigBase>(config));
}
