/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/config.h"
#include "configcpp/config_object.h"

using namespace config;

static std::string resourcePath() {
	static std::string path;
    if (!path.empty()) {
        return path;
    }
    path = "../../test/resources";
    if (MiscUtils::fileExists(path)) {
        return path;
    }
    path = "../test/resources";
    if (MiscUtils::fileExists(path)) {
        return path;
    }
    path = "./test/resources";
    if (MiscUtils::fileExists(path)) {
        return path;
    }
    throw std::runtime_error("test/resources directory not found");
}

int main(int argc, char** argv) {
	auto conf = Config::load(resourcePath() + "/test01");

    // you don't have to write the types explicitly of course,
    // just doing that to show what they are.
    int32_t a = conf->getInt("ints.fortyTwo");
    auto child = conf->getConfig("ints");
    int32_t b = child->getInt("fortyTwo");
    int64_t ms = conf->getMilliseconds("durations.halfSecond");

    // a Config has an associated tree of values, with a ConfigObject
    // at the root. The ConfigObject implements std::unordered_map
    auto obj = conf->root();

    // this is how you do conf->getInt "manually" on the value tree, if you
    // were so inclined. (This is not a good approach vs. conf->getInt() above,
    // just showing how ConfigObject stores a tree of values.)
    int32_t c = std::dynamic_pointer_cast<ConfigObject>(obj->get("ints"))->get("fortyTwo")->unwrapped<int32_t>();

    // you can manually unwrap variant values as follows
    int32_t d = variant_get<int32_t>(conf->getVariant("ints.fortyTwo"));

    std::cout << "Config example values:" << std::endl;
    std::cout << "a:" << a << std::endl;
    std::cout << "b:" << b << std::endl;
    std::cout << "ms:" << ms << std::endl;
    std::cout << "c:" << c << std::endl;
    std::cout << "d:" << d << std::endl;
}
