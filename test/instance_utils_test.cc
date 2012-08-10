/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "test_fixture.h"
#include "configcpp/detail/abstract_config_object.h"
#include "configcpp/detail/path.h"
#include "configcpp/detail/file_reader.h"
#include "configcpp/detail/token.h"
#include "configcpp/detail/simple_config_object.h"

using namespace config;

class InstanceUtilsTest : public TestFixture {
};

TEST_F(InstanceUtilsTest, instanceClass) {
    auto ptr = Path::newPath("/tmp");
    EXPECT_TRUE(instanceof<Path>(ptr));
    EXPECT_TRUE(instanceof<ConfigBase>(ptr));
    EXPECT_FALSE(instanceof<FileReader>(ptr));
    EXPECT_FALSE(instanceof<FileReader>(ConfigValuePtr()));
    EXPECT_FALSE(instanceof<ConfigBase>(ConfigBasePtr()));
}

TEST_F(InstanceUtilsTest, instanceVariantScalar) {
    ConfigVariant var = (int32_t)123;
    EXPECT_TRUE(instanceof<int32_t>(var));
    EXPECT_FALSE(instanceof<int64_t>(var));
    EXPECT_FALSE(instanceof<std::string>(var));
    EXPECT_FALSE(instanceof<ConfigBase>(var));
    EXPECT_FALSE(instanceof<VectorVariant>(var));
}

TEST_F(InstanceUtilsTest, instanceVariantConfigBase) {
    ConfigVariant var = SimpleConfigObject::makeEmpty();
    EXPECT_TRUE(instanceof<ConfigBase>(var));
    EXPECT_TRUE(instanceof<ConfigValue>(var));
    EXPECT_TRUE(instanceof<SimpleConfigObject>(var));
    EXPECT_FALSE(instanceof<Token>(var));
    EXPECT_FALSE(instanceof<int32_t>(var));
    EXPECT_FALSE(instanceof<std::string>(var));
    EXPECT_FALSE(instanceof<VectorVariant>(var));
}

TEST_F(InstanceUtilsTest, instanceVariantVector) {
    ConfigVariant var = VectorVariant({(int32_t)1, (int32_t)2});
    EXPECT_TRUE(instanceof<VectorVariant>(var));
    EXPECT_FALSE(instanceof<MapVariant>(var));
    EXPECT_FALSE(instanceof<ConfigBase>(var));
    EXPECT_FALSE(instanceof<ConfigValue>(var));
    EXPECT_FALSE(instanceof<Path>(var));
    EXPECT_FALSE(instanceof<Token>(var));
    EXPECT_FALSE(instanceof<int32_t>(var));
    EXPECT_FALSE(instanceof<std::string>(var));
}
