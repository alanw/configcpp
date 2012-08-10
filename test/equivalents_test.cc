/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "test_fixture.h"
#include "configcpp/detail/abstract_config_object.h"
#include "configcpp/detail/resolve_context.h"
#include "configcpp/config_syntax.h"
#include "configcpp/config_parse_options.h"
#include "configcpp/config_resolve_options.h"
#include "configcpp/config.h"

using namespace config;

class EquivalentsTest : public TestFixture {
protected:
    VectorString equiv01_dir() {
        static VectorString _equiv01_dir = {
            "comments.conf",
            "equals.conf",
            "no-commas.conf",
            "no-root-braces.conf",
            "no-whitespace.json",
            "omit-colons.conf",
            "path-keys.conf",
            "properties-style.conf",
            "substitutions.conf",
            "unquoted.conf"
        };
        return _equiv01_dir;
    }

    std::string equiv01_orig() {
        return "original.json";
    }

    VectorString equiv02_dir() {
        static VectorString _equiv02_dir = {
            "path-keys-weird-whitespace.conf",
            "path-keys.conf"
        };
        return _equiv02_dir;
    }

    std::string equiv02_orig() {
        return "original.json";
    }

    VectorString equiv03_dir() {
        static VectorString _equiv03_dir = {
            "includes.conf"
        };
        return _equiv03_dir;
    }

    std::string equiv03_orig() {
        return "original.json";
    }

    VectorString equiv04_dir() {
        static VectorString _equiv04_dir = {
            "missing-substitutions.conf"
        };
        return _equiv04_dir;
    }

    std::string equiv04_orig() {
        return "original.json";
    }

    VectorString equiv05_dir() {
        static VectorString _equiv05_dir = {
            "triple-quotes.conf"
        };
        return _equiv05_dir;
    }

    std::string equiv05_orig() {
        return "original.json";
    }

    AbstractConfigValuePtr postParse(const ConfigValuePtr& value) {
        if (instanceof<AbstractConfigObject>(value)) {
            // for purposes of these tests, substitutions are only
            // against the same file's root, and without looking at
            // system prop or env variable fallbacks.
            return ResolveContext::resolve(std::dynamic_pointer_cast<AbstractConfigValue>(value), std::dynamic_pointer_cast<AbstractConfigObject>(value), ConfigResolveOptions::noSystem());
        }
        return std::dynamic_pointer_cast<AbstractConfigValue>(value);
    }

    AbstractConfigValuePtr parse(ConfigSyntax flavor, const std::string& f) {
        auto options = ConfigParseOptions::defaults()->setSyntax(flavor);
        return postParse(Config::parseFile(f, options)->root());
    }

    AbstractConfigValuePtr parse(const std::string& f) {
        auto options = ConfigParseOptions::defaults();
        return postParse(Config::parseFile(f, options)->root());
    }

    void testEquiv(const std::string& path, const std::string& orig, const VectorString& dir) {
        auto original = parse(path + orig);
        for (auto& testFile : dir) {
            auto value = parse(path + testFile);
            checkEquals(original, value, path + testFile);

            // check that all .json files can be parsed as .conf,
            // i.e. .conf must be a superset of JSON
            if (boost::ends_with(testFile, ".json")) {
                auto parsedAsConf = parse(ConfigSyntax::CONF, path + testFile);
                checkEquals(original, parsedAsConf, path + testFile);
            }
        }
    }
};

TEST_F(EquivalentsTest, testEquivalents) {
    testEquiv(resourcePath() + "/equiv01/", equiv01_orig(), equiv01_dir());
    testEquiv(resourcePath() + "/equiv02/", equiv02_orig(), equiv02_dir());
    testEquiv(resourcePath() + "/equiv03/", equiv03_orig(), equiv03_dir());
    testEquiv(resourcePath() + "/equiv04/", equiv04_orig(), equiv04_dir());
    testEquiv(resourcePath() + "/equiv05/", equiv05_orig(), equiv05_dir());
}
