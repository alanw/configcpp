/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "test_fixture.h"
#include "configcpp/detail/path.h"
#include "configcpp/detail/config_int.h"
#include "configcpp/detail/config_int64.h"
#include "configcpp/detail/config_boolean.h"
#include "configcpp/detail/config_null.h"
#include "configcpp/detail/config_string.h"
#include "configcpp/detail/config_double.h"
#include "configcpp/detail/config_reference.h"
#include "configcpp/detail/config_concatenation.h"
#include "configcpp/detail/simple_config_origin.h"
#include "configcpp/detail/abstract_config_object.h"
#include "configcpp/detail/substitution_expression.h"
#include "configcpp/detail/tokens.h"
#include "configcpp/detail/tokenizer.h"
#include "configcpp/detail/string_reader.h"
#include "configcpp/config.h"
#include "configcpp/config_parse_options.h"
#include "configcpp/config_syntax.h"

namespace config {

bool NotEqualToAnythingElse::equals(const ConfigVariant& other) {
    return instanceof<NotEqualToAnythingElse>(other);
}

uint32_t NotEqualToAnythingElse::hashCode() {
    return 971;
}

void TestFixture::checkSame(const ConfigBasePtr& a, const ConfigBasePtr& b, const std::string& desc) {
    std::string msg = desc.empty() ? "" : " [" + desc + "]";
    EXPECT_EQ(a.get(), b.get()) << a.get() << " != " << b.get() << msg;
}

void TestFixture::checkNotSame(const ConfigBasePtr& a, const ConfigBasePtr& b, const std::string& desc) {
    std::string msg = desc.empty() ? "" : " [" + desc + "]";
    EXPECT_NE(a.get(), b.get()) << a.get() << " == " << b.get() << msg;
}

void TestFixture::checkEquals(const ConfigBasePtr& a, const ConfigBasePtr& b, const std::string& desc) {
    std::string msg = desc.empty() ? "" : " [" + desc + "]";
    EXPECT_TRUE(a->equals(b)) << a->toString() << " != " << b->toString() << msg;
}

void TestFixture::checkNotEquals(const ConfigBasePtr& a, const ConfigBasePtr& b, const std::string& desc) {
    std::string msg = desc.empty() ? "" : " [" + desc + "]";
    EXPECT_FALSE(a->equals(b)) << a->toString() << " == " << b->toString() << msg;
}

void TestFixture::checkNotEqualToRandomOtherThing(const ConfigBasePtr& a, const std::string& desc) {
    std::string msg = desc.empty() ? "" : " [" + desc + "]";
    static const auto notEqualToAnything = NotEqualToAnythingElse::make_instance();
    EXPECT_FALSE(a->equals(notEqualToAnything)) << msg;
    EXPECT_FALSE(notEqualToAnything->equals(a)) << msg;
}

void TestFixture::checkEqualObjects(const ConfigBasePtr& a, const ConfigBasePtr& b, const std::string& desc) {
    std::string msg = desc.empty() ? "" : " [" + desc + "]";
    checkEquals(a, b, desc);
    checkEquals(b, a, desc);
    EXPECT_EQ(a->hashCode(), b->hashCode()) << msg;
    checkNotEqualToRandomOtherThing(a, desc);
    checkNotEqualToRandomOtherThing(b, desc);
}

void TestFixture::checkNotEqualObjects(const ConfigBasePtr& a, const ConfigBasePtr& b, const std::string& desc) {
    std::string msg = desc.empty() ? "" : " [" + desc + "]";
    checkNotEquals(a, b, desc);
    checkNotEquals(b, a, desc);
    // hashcode inequality isn't guaranteed, but
    // as long as it happens to work it might
    // detect a bug (if hashcodes are equal,
    // check if it's due to a bug or correct
    // before you remove this)
    EXPECT_NE(a->hashCode(), b->hashCode()) << msg;
    checkNotEqualToRandomOtherThing(a, desc);
    checkNotEqualToRandomOtherThing(b, desc);
}

std::string TestFixture::resourcePath() {
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

PathPtr TestFixture::path(const VectorString& elements) {
    return Path::make_instance(elements);
}

SimpleConfigOriginPtr TestFixture::fakeOrigin() {
    return SimpleConfigOrigin::newSimple("fake origin");
}

VectorParseTest TestFixture::invalidJsonInvalidConf() {
    static VectorParseTest _invalidJsonInvalidConf = {
        {"{", false},
        {"}", false},
        {"[", false},
        {"]", false},
        {", false},", false},
        {"10", false}, // value not in array or object
        {"\"foo\"", false}, // value not in array or object
        {"\"", false}, // single quote by itself
        // {"[,]", false}, // array with just a comma in it; lift is OK with this
        // {"[,,]", false}, // array with just two commas in it; lift is cool with this too
        // {"[1,2,,]", false}, // array with two trailing commas
        // {"[,1,2]", false}, // array with initial comma
        // {"{ , }", false}, // object with just a comma in it
        // {"{ , , }", false}, // object with just two commas in it
        {"{ 1,2 }", false}, // object with single values not key-value pair
        // {"{ , \"foo\" : 10 }", false}, // object starts with comma
        // {"{ \"foo\" : 10 ,, }", false}, // object has two trailing commas
        {" \"a\" : 10 ,, ", false}, // two trailing commas for braceless root object
        {"{ \"foo\" : }", false}, // no value in object
        {"{ : 10 }", false}, // no key in object
        {" \"foo\" : ", false}, // no value in object with no braces
        {" : 10 ", false}, // no key in object with no braces
        {" \"foo\" : 10 } ", false}, // close brace but no open
        {" \"foo\" : 10 [ ", false}, // no-braces object with trailing gunk
        {"{ \"foo\" }", false}, // no value or colon
        {"{ \"a\" : [ }", false}, // [ is not a valid value
        {"{ \"foo\" : 10, true }", false}, // non-key after comma
        {"{ foo \n bar : 10 }", false}, // newline in the middle of the unquoted key
        {"[ 1, \\", false}, // ends with backslash
        // these two problems are ignored by the lift tokenizer
        {"[:\"foo\", false}, \"bar\"]", false}, // colon in an array; lift doesn't throw (tokenizer erases it)
        {"[\"foo\" : \"bar\"]", false}, // colon in an array another way, lift ignores (tokenizer erases it)
        {"[ 10e3e3 ]", false}, // two exponents. ideally this might parse to a number plus string "e3" but it's hard to implement.
        {"[ 1-e3 ]", false}, // malformed number but all chars can appear in a number
        {"[ \"hello ]", false}, // unterminated string
        // {"{ \"foo\" , true }", false}, // comma instead of colon, lift is fine with this
        // {"{ \"foo\" : true \"bar\" : false }", false}, // missing comma between fields, lift fine with this
        {"[ 10, }]", false}, // array with } as an element
        {"[ 10, {]", false}, // array with { as an element
        {"{}x", false}, // trailing invalid token after the root object
        {"[]x", false}, // trailing invalid token after the root array
        // {"{}{}", false}, // trailing token after the root object - lift OK with it
        {"{}true", false}, // trailing token after the root object
        // {"[]{}", false}, // trailing valid token after the root array
        {"[]true", false}, // trailing valid token after the root array
        {"[${]", false}, // unclosed substitution
        {"[$]", false}, // '$' by itself
        {"[$  ]", false}, // '$' by itself with spaces after
        {"[${}]", false}, // empty substitution (no path)
        {"[${?}]", false}, // no path with ? substitution
        {"[${ ?foo}]", true}, // space before ? not allowed
        {"{ \"a\" : [1,2], \"b\" : y${a}z }", false}, // trying to interpolate an array in a string
        {"{ \"a\" : { \"c\" : 2 }, \"b\" : y${a}z }", false}, // trying to interpolate an object in a string
        {"{ \"a\" : ${a} }", false}, // simple cycle
        {"[ { \"a\" : 2, \"b\" : ${${a}} } ]", false}, // nested substitution
        {"[ = ]", false}, // = is not a valid token in unquoted text
        {"[ + ]", false},
        {"[ # ]", false},
        {"[ ` ]", false},
        {"[ ^ ]", false},
        {"[ ? ]", false},
        {"[ ! ]", false},
        {"[ @ ]", false},
        {"[ * ]", false},
        {"[ & ]", false},
        {"[ \\ ]", false},
        {"+=", false},
        {"[ += ]", false},
        {"+= 10", false},
        {"10 +=", false},
        // {"[ \"foo\nbar\" ]", false}, // unescaped newline in quoted string, lift doesn't care
        {"[ # comment ]", false},
        {"${ #comment }", false},
        {"[ // comment ]", false},
        {"${ // comment }", false},
        {"{ include \"bar\" : 10 }", false}, // include with a value after it
        {"{ include foo }", false}, // include with unquoted string
        {"{ include : { \"a\" : 1 } }", false} // include used as unquoted key
    };
    return _invalidJsonInvalidConf;
}

VectorParseTest TestFixture::validJson() {
    static VectorParseTest _validJson = {
        {"[]", false},
        {"{ \"foo\" : \"bar\" }}", false},
        {"\"foo\",\"bar\"", false},
        {"{ \"foo\" : 42 }", false},
        {"{ \"foo\"\n : 42 }", false}, // newline after key
        {"{ \"foo\" : \n 42 }", false}, // newline after colon
        {"[10, 11]", false},
        {"[10,\"foo\"]", false},
        {"{ \"foo\" : \"bar\", \"baz\" : \"boo\" }", false},
        {"{ \"foo\" : { \"bar\" : \"baz\" }, \"baz\" : \"boo\" }", false},
        {"{ \"foo\" : { \"bar\" : \"baz\", \"woo\" : \"w00t\" }, \"baz\" : \"boo\" }", false},
        {"{ \"foo\" : [10,11,12], \"baz\" : \"boo\" }", false},
        {"[{},{},{},{}]", false},
        {"[[[[[[]]]]]]", false},
        {"[[1], [1,2], [1,2,3], []]", false}, // nested multiple-valued array
        {"{\"a\":{\"a\":{\"a\":{\"a\":{\"a\":{\"a\":{\"a\":{\"a\":42}}}}}}}}", false},
        {"[ \"#comment\" ]", false}, // quoted # comment
        {"[ \"//comment\" ]", false}, // quoted // comment
        // this long one is mostly to test rendering
        {"{ \"foo\" : { \"bar\" : \"baz\", \"woo\" : \"w00t\" }, \"baz\" : { \"bar\" : \"baz\", \"woo\" : [1,2,3,4], \"w00t\" : true, \"a\" : false, \"b\" : 3.14, \"c\" : null } }", false},
        {"{}", false}
    };
    return _validJson;
}

VectorParseTest TestFixture::validConfInvalidJson() {
    static VectorParseTest _validConfInvalidJson = {
        {" ", false}, // empty document single space
        {"\n", false}, // empty document single newline
        {" \n \n   \n\n\n", false}, // complicated empty document
        {"# foo", false}, // just a comment
        {"# bar\n", false}, // just a comment with a newline
        {"# foo\n//bar", false}, // comment then another with no newline
        {"{ \"foo\" = 42 }", false}, // equals rather than colon
        {"{ foo { \"bar\" : 42 } }", false}, // omit the colon for object value
        {"{ foo baz { \"bar\" : 42 } }", false}, // omit the colon with unquoted key with spaces
        {" \"foo\" : 42 ", false}, // omit braces on root object
        {"{ \"foo\" : bar }", false}, // no quotes on value
        {"{ \"foo\" : null bar 42 baz true 3.14 \"hi\" }", false}, // bunch of values to concat into a string
        {"{ foo : \"bar\" }", false}, // no quotes on key
        {"{ foo : bar }", false}, // no quotes on key or value
        {"{ foo.bar : bar }", false}, // path expression in key
        {"{ foo.\"hello world\".baz : bar }", false}, // partly-quoted path expression in key
        {"{ foo.bar \n : bar }", false}, // newline after path expression in key
        {"{ foo  bar : bar }", false}, // whitespace in the key
        {"{ true : bar }", false}, // key is a non-string token
        // {"{ "foo" : "bar", "foo" : "bar2" }", false}, // dup keys - lift just returns both
        // {"[ 1, 2, 3, ]", false}, // single trailing comma (lift fails to throw)
        // {"[1,2,3  , ]", false}, // single trailing comma with whitespace
        // {"[1,2,3\n\n , \n]", false}, // single trailing comma with newlines
        // {"[1,]", false}, // single trailing comma with one-element array
        // {"{ \"foo\" : 10, }", false}, // extra trailing comma (lift fails to throw)
        // {"{ \"a\" : \"b\", }", false}, // single trailing comma in object
        {"{ a : b, }", false}, // single trailing comma in object (unquoted strings)
        {"{ a : b  \n  , \n }", false}, // single trailing comma in object with newlines
        {"a : b, c : d,", false}, // single trailing comma in object with no root braces
        {"{ a : b\nc : d }", false}, // skip comma if there's a newline
        {"a : b\nc : d", false}, // skip comma if there's a newline and no root braces
        {"a : b\nc : d,", false}, // skip one comma but still have one at the end
        {"[ foo ]", false}, // not a known token in JSON
        {"[ t ]", false}, // start of "true" but ends wrong in JSON
        {"[ tx ]", false},
        {"[ tr ]", false},
        {"[ trx ]", false},
        {"[ tru ]", false},
        {"[ trux ]", false},
        {"[ truex ]", false},
        {"[ 10x ]", false}, // number token with trailing junk
        {"[ / ]", false}, // unquoted string "slash"
        {"{ include \"foo\" }", false}, // valid include
        {"{ include\n\"foo\" }", false}, // include with just a newline separating from string
        {"{ include\"foo\" }", false}, // include with no whitespace after it
        {"[ include ]", false}, // include can be a string value in an array
        {"{ foo : include }", false}, // include can be a field value also
        {"{ include \"foo\", \"a\" : \"b\" }", false}, // valid include followed by comma and field
        {"{ foo include : 42 }", false}, // valid to have a key not starting with include
        {"[ ${foo} ]", false},
        {"[ ${?foo} ]", false},
        {"[ ${\"foo\"} ]", false},
        {"[ ${foo.bar} ]", false},
        {"[ abc  xyz  ${foo.bar}  qrs tuv ]", false}, // value concatenation
        {"[ 1, 2, 3, blah ]", false},
        {"[ ${\"foo.bar\"} ]", false},
        {"{} # comment", false},
        {"{} // comment", false},
        {"{ \"foo\" #comment\n: 10 }", false},
        {"{ \"foo\" // comment\n: 10 }", false},
        {"{ \"foo\" : #comment\n 10 }", false},
        {"{ \"foo\" : // comment\n 10 }", false},
        {"{ \"foo\" : 10 #comment\n }", false},
        {"{ \"foo\" : 10 // comment\n }", false},
        {"[ 10, # comment\n 11]", false},
        {"[ 10, // comment\n 11]", false},
        {"[ 10 # comment\n, 11]", false},
        {"[ 10 // comment\n, 11]", false},
        {"{ /a/b/c : 10 }", false}, // key has a slash in it
        {"[${ foo.bar}]", true}, // substitution with leading spaces
        {"[${foo.bar }]", true}, // substitution with trailing spaces
        {"[${ \"foo.bar\"}]", true}, // substitution with leading spaces and quoted
        {"[${\"foo.bar\" }]", true}, // substitution with trailing spaces and quoted
        {"[ ${\"foo\"\"bar\"} ]", false}, // multiple strings in substitution
        {"[ ${foo  \"bar\"  baz} ]", false}, // multiple strings and whitespace in substitution
        {"[${true}]", false}, // substitution with unquoted true token
        {"a = [], a += b", false}, // += operator with previous init
        {"{ a = [], a += 10 }", false}, // += in braces object with previous init
        {"a += b", false}, // += operator without previous init
        {"{ a += 10 }", false} // += in braces object without previous init
    };
    return _validConfInvalidJson;
}

VectorParseTest TestFixture::invalidJson() {
    static VectorParseTest _invalidJson;
    if (_invalidJson.empty()) {
        _invalidJson = validConfInvalidJson();
        _invalidJson.insert(_invalidJson.end(), invalidJsonInvalidConf().begin(), invalidJsonInvalidConf().end());
    }
    return _invalidJson;
}

VectorParseTest TestFixture::invalidConf() {
    static VectorParseTest _invalidConf = invalidJsonInvalidConf();
    return _invalidConf;
}

VectorParseTest TestFixture::validConf() {
    // .conf is a superset of JSON so validJson just goes in here
    static VectorParseTest _validConf;
    if (_validConf.empty()) {
        _validConf = validConfInvalidJson();
        _validConf.insert(_validConf.end(), validJson().begin(), validJson().end());
    }
    return _validConf;
}

VectorString TestFixture::whitespaceVariations(const VectorParseTest& tests) {
    VectorString variations;
    for (auto& test : tests) {
        variations.push_back(test.first);
        if (test.second) {
            continue;
        }
        variations.push_back(test.first);
        variations.push_back(" " + test.first);
        variations.push_back(test.first + " ");
        variations.push_back(" " + test.first + " ");
        variations.push_back(boost::replace_all_copy(test.first, " ", "")); // this would break with whitespace in a key or value
        variations.push_back(boost::replace_all_copy(test.first, ":", " : ")); // could break with : in a key or value
        variations.push_back(boost::replace_all_copy(test.first, ",", " , ")); // could break with , in a key or value
    }
    return variations;
}

AbstractConfigValuePtr TestFixture::intValue(int32_t i) {
    return ConfigInt::make_instance(fakeOrigin(), i, "");
}

AbstractConfigValuePtr TestFixture::int64Value(int64_t l) {
    return ConfigInt64::make_instance(fakeOrigin(), l, "");
}

AbstractConfigValuePtr TestFixture::boolValue(bool b) {
    return ConfigBoolean::make_instance(fakeOrigin(), b);
}

AbstractConfigValuePtr TestFixture::nullValue() {
    return ConfigNull::make_instance(fakeOrigin());
}

AbstractConfigValuePtr TestFixture::stringValue(const std::string& s) {
    return ConfigString::make_instance(fakeOrigin(), s);
}

AbstractConfigValuePtr TestFixture::doubleValue(double d) {
    return ConfigDouble::make_instance(fakeOrigin(), d, "");
}

AbstractConfigObjectPtr TestFixture::parseObject(const std::string& s) {
    return std::dynamic_pointer_cast<AbstractConfigObject>(parseConfig(s)->root());
}

ConfigPtr TestFixture::parseConfig(const std::string& s) {
    auto options = ConfigParseOptions::defaults()->setOriginDescription("test string")->setSyntax(ConfigSyntax::CONF);
    return Config::parseString(s, options);
}


ConfigReferencePtr TestFixture::subst(const std::string& ref, bool optional) {
    auto path = Path::newPath(ref);
    return ConfigReference::make_instance(fakeOrigin(), SubstitutionExpression::make_instance(path, optional));
}

AbstractConfigValuePtr TestFixture::substInString(const std::string& ref, bool optional) {
    auto path = Path::newPath(ref);
    VectorAbstractConfigValue pieces = {stringValue("start<"), subst(ref, optional), stringValue(">end")};
    return ConfigConcatenation::make_instance(fakeOrigin(), pieces);
}

TokenPtr TestFixture::tokenTrue() {
    return Tokens::newBoolean(fakeOrigin(), true);
}

TokenPtr TestFixture::tokenFalse() {
    return Tokens::newBoolean(fakeOrigin(), false);
}

TokenPtr TestFixture::tokenNull() {
    return Tokens::newNull(fakeOrigin());
}

TokenPtr TestFixture::tokenUnquoted(const std::string& s) {
    return Tokens::newUnquotedText(fakeOrigin(), s);
}

TokenPtr TestFixture::tokenString(const std::string& s) {
    return Tokens::newString(fakeOrigin(), s);
}

TokenPtr TestFixture::tokenDouble(double d) {
    return Tokens::newDouble(fakeOrigin(), d, "");
}

TokenPtr TestFixture::tokenInt(int32_t i) {
    return Tokens::newInt(fakeOrigin(), i, "");
}

TokenPtr TestFixture::tokenInt64(int64_t i) {
    return Tokens::newInt64(fakeOrigin(), i, "");
}

TokenPtr TestFixture::tokenLine(int32_t line) {
    return Tokens::newLine(fakeOrigin()->setLineNumber(line));
}

TokenPtr TestFixture::tokenComment(const std::string& text) {
    return Tokens::newComment(fakeOrigin(), text);
}

TokenPtr TestFixture::tokenMaybeOptionalSubstitution(bool optional, const VectorToken& expression) {
    return Tokens::newSubstitution(fakeOrigin(), optional, expression);
}

TokenPtr TestFixture::tokenSubstitution(const VectorToken& expression) {
    return tokenMaybeOptionalSubstitution(false, expression);
}

TokenPtr TestFixture::tokenOptionalSubstitution(const VectorToken& expression) {
    return tokenMaybeOptionalSubstitution(true, expression);
}

TokenPtr TestFixture::tokenKeySubstitution(const std::string& s) {
    return tokenSubstitution(VectorToken({tokenString(s)}));
}

TokenIteratorPtr TestFixture::tokenize(const ConfigOriginPtr& origin, const ReaderPtr& input) {
    return Tokenizer::tokenize(origin, input, ConfigSyntax::CONF);
}

TokenIteratorPtr TestFixture::tokenize(const ReaderPtr& input) {
    return tokenize(SimpleConfigOrigin::newSimple("anonymous Reader"), input);
}

TokenIteratorPtr TestFixture::tokenize(const std::string& s) {
    return tokenize(StringReader::make_instance(s));
}

VectorToken TestFixture::tokenizeAsList(const std::string& s) {
    VectorToken tokens;
    for (auto token = tokenize(s); token->hasNext(); ) {
        tokens.push_back(token->next());
    }
    return tokens;
}

}
