/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef PARSER_H_
#define PARSER_H_

#include "configcpp/detail/config_base.h"
#include "configcpp/config_exception.h"

namespace config {

class Parser : public ConfigBase {
public:
    CONFIG_CLASS(Parser);

    static AbstractConfigValuePtr parse(const TokenIteratorPtr& tokens,
                                        const ConfigOriginPtr& origin,
                                        const ConfigParseOptionsPtr& options,
                                        const ConfigIncludeContextPtr& includeContext);

private:
    static void addPathText(VectorElement& buf, bool wasQuoted, const std::string& newText);

    static PathPtr parsePathExpression(const TokenIteratorPtr& expression,
                                       const ConfigOriginPtr& origin,
                                       const std::string& originalText = "");

public:
    static PathPtr parsePath(const std::string& path);

private:
    /// The idea is to see if the string has any chars that might require the
    /// full parser to deal with.
    static bool hasUnsafeChars(const std::string& s);

    static void appendPathString(const PathBuilderPtr& pb, const std::string& s);

    /// Do something much faster than the full parser if
    /// we just have something like "foo" or "foo.bar"
    static PathPtr speculativeFastParsePath(const std::string& path);

    friend class ParseContext;
};

class TokenWithComments : public ConfigBase {
public:
    CONFIG_CLASS(TokenWithComments);

    TokenWithComments(const TokenPtr& token, const VectorToken& comments = VectorToken());

    TokenWithCommentsPtr prepend(const VectorToken& earlier);
    SimpleConfigOriginPtr setComments(const SimpleConfigOriginPtr& origin);

    virtual std::string toString() override;

private:
    TokenPtr token;
    VectorToken comments;

    friend class ParseContext;
};

class ParseContext : public ConfigBase {
public:
    CONFIG_CLASS(ParseContext);

    ParseContext(ConfigSyntax flavor, const ConfigOriginPtr& origin,
                 const TokenIteratorPtr& tokens, const FullIncluderPtr& includer,
                 const ConfigIncludeContextPtr& includeContext);

private:
    void consolidateCommentBlock(const TokenPtr& commentToken);
    TokenWithCommentsPtr popToken();
    TokenWithCommentsPtr nextToken();
    void putBack(const TokenWithCommentsPtr& token);
    TokenWithCommentsPtr nextTokenIgnoringNewline();
    bool checkElementSeparator();

    // Merge a bunch of adjacent values into one
    // value; change unquoted text into a string
    // value.
    void consolidateValueTokens();

    static SubstitutionExpressionPtr tokenToSubstitutionExpression(const TokenPtr& valueToken);

    ConfigOriginPtr lineOrigin();

    ConfigExceptionParse parseError(const std::string& message);

    std::string previousFieldName(const PathPtr& lastPath);
    PathPtr fullCurrentPath();
    std::string previousFieldName();

    std::string addKeyName(const std::string& message);
    std::string addQuoteSuggestion(const std::string& badToken,
                                   const std::string& message);
    std::string addQuoteSuggestion(const PathPtr& lastPath,
                                   bool insideEquals,
                                   const std::string& badToken,
                                   const std::string& message);

    AbstractConfigValuePtr parseValue(const TokenWithCommentsPtr& t);

    static AbstractConfigObjectPtr createValueUnderPath(const PathPtr& path,
                                                        const AbstractConfigValuePtr& value);

    PathPtr parseKey(const TokenWithCommentsPtr& token);

    static bool isIncludeKeyword(const TokenPtr& t);
    static bool isUnquotedWhitespace(const TokenPtr& t);

    void parseInclude(MapAbstractConfigValue& values);

    bool isKeyValueSeparatorToken(const TokenPtr& t);

    AbstractConfigObjectPtr parseObject(bool hadOpenCurly);
    SimpleConfigListPtr parseArray();

public:
    AbstractConfigValuePtr parse();

private:
    int32_t lineNumber;
    StackTokenWithComments buffer;
    TokenIteratorPtr tokens;
    FullIncluderPtr includer;
    ConfigIncludeContextPtr includeContext;
    ConfigSyntax flavor;
    ConfigOriginPtr baseOrigin;
    StackPath pathStack;
    int32_t equalsCount;
};

class Element : public ConfigBase {
public:
    CONFIG_CLASS(Element);

    Element(const std::string& initial, bool canBeEmpty);

    virtual std::string toString() override;

    std::string element;
    // an element can be empty if it has a quoted empty string "" in it
    bool canBeEmpty;
};

}

#endif // PARSER_H_
