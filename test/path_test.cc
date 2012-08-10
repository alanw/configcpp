/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "test_fixture.h"
#include "configcpp/detail/path.h"
#include "configcpp/detail/parser.h"

using namespace config;

class PathTest : public TestFixture {
};

TEST_F(PathTest, pathEquality) {
    // note: foo.bar is a single key here
    auto a = Path::newKey("foo.bar");
    // check that newKey worked
    checkEquals(path({"foo.bar"}), a);
    auto sameAsA = Path::newKey("foo.bar");
    auto differentKey = Path::newKey("hello");
    // here foo.bar is two elements
    auto twoElements = Path::newPath("foo.bar");
    // check that newPath worked
    checkEquals(path({"foo", "bar"}), twoElements);
    auto sameAsTwoElements = Path::newPath("foo.bar");

    checkEqualObjects(a, a);
    checkEqualObjects(a, sameAsA);
    checkNotEqualObjects(a, differentKey);
    checkNotEqualObjects(a, twoElements);
    checkEqualObjects(twoElements, sameAsTwoElements);
}

TEST_F(PathTest, pathToString) {
    EXPECT_EQ("Path(foo)", path({"foo"})->toString());
    EXPECT_EQ("Path(foo.bar)", path({"foo", "bar"})->toString());
    EXPECT_EQ("Path(foo.\"bar*\")", path({"foo", "bar*"})->toString());
    EXPECT_EQ("Path(\"foo.bar\")", path({"foo.bar"})->toString());
}

TEST_F(PathTest, pathRender) {
    std::vector<std::pair<std::string, PathPtr>> tests({
        // simple one-element case
        {"foo", path({"foo"})},
        // simple two-element case
        {"foo.bar", path({"foo", "bar"})},
        // non-safe-char in an element
        {"foo.\"bar*\"", path({"foo", "bar*"})},
        // period in an element
        {"\"foo.bar\"", path({"foo.bar"})},
        // hyphen and underscore
        {"foo-bar", path({"foo-bar"})},
        {"foo_bar", path({"foo_bar"})},
        // starts with hyphen
        {"\"-foo\"", path({"-foo"})},
        // starts with number
        {"\"10foo\"", path({"10foo"})},
        // empty elements
        {"\"\".\"\"", path({"", ""})},
        // internal space
        {"\"foo bar\"", path({"foo bar"})},
        // leading and trailing spaces
        {"\" foo \"", path({" foo "})},
        // trailing space only
        {"\"foo \"", path({"foo "})}
    });

    for (auto& t : tests) {
        EXPECT_EQ(t.first, t.second->render());
        checkEquals(t.second, Parser::parsePath(t.first));
        checkEquals(t.second, Parser::parsePath(t.second->render()));
    }
}

TEST_F(PathTest, pathFromPathList) {
    checkEquals(path({"foo"}), Path::make_instance(VectorPath({path({"foo"})})));
    checkEquals(path({"foo", "bar", "baz", "boo"}), Path::make_instance(VectorPath({path({"foo", "bar", "baz", "boo"})})));
}

TEST_F(PathTest, pathPrepend) {
    checkEquals(path({"foo", "bar"}), path({"bar"})->prepend(path({"foo"})));
    checkEquals(path({"a", "b", "c", "d"}), path({"c", "d"})->prepend(path({"a", "b"})));
}

TEST_F(PathTest, pathLength) {
    EXPECT_EQ(1, path({"foo"})->length());
    EXPECT_EQ(2, path({"foo", "bar"})->length());
}

TEST_F(PathTest, pathParent) {
    EXPECT_FALSE(path({"a"})->parent());
    checkEquals(path({"a"}), path({"a", "b"})->parent());
    checkEquals(path({"a", "b"}), path({"a", "b", "c"})->parent());
}

TEST_F(PathTest, pathLast) {
    EXPECT_EQ("a", path({"a"})->last());
    EXPECT_EQ("b", path({"a", "b"})->last());
}

TEST_F(PathTest, pathsAreInvalid) {
    // this test is just of the Path.newPath() wrapper, the extensive
    // test of different paths is over in ConfParserTest
    EXPECT_THROW(Path::newPath(""), ConfigExceptionBadPath);
    EXPECT_THROW(Path::newPath(".."), ConfigExceptionBadPath);
}
