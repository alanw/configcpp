/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef TOKENS_H_
#define TOKENS_H_

#include "configcpp/detail/token.h"

namespace config {

class Tokens : public ConfigBase {
public:
    CONFIG_CLASS(Tokens);

    static bool isValue(const TokenPtr& token);
    static AbstractConfigValuePtr getValue(const TokenPtr& token);
    static bool isValueWithType(const TokenPtr& t, ConfigValueType valueType);
    static bool isNewline(const TokenPtr& token);
    static bool isProblem(const TokenPtr& token);
    static std::string getProblemWhat(const TokenPtr& token);
    static std::string getProblemMessage(const TokenPtr& token);
    static bool getProblemSuggestQuotes(const TokenPtr& token);
    static bool isComment(const TokenPtr& token);
    static std::string getCommentText(const TokenPtr& token);
    static bool isUnquotedText(const TokenPtr& token);
    static std::string getUnquotedText(const TokenPtr& token);
    static bool isSubstitution(const TokenPtr& token);
    static VectorToken getSubstitutionPathExpression(const TokenPtr& token);
    static bool getSubstitutionOptional(const TokenPtr& token);

    static TokenPtr START();
    static TokenPtr END();
    static TokenPtr COMMA();
    static TokenPtr EQUALS();
    static TokenPtr COLON();
    static TokenPtr OPEN_CURLY();
    static TokenPtr CLOSE_CURLY();
    static TokenPtr OPEN_SQUARE();
    static TokenPtr CLOSE_SQUARE();
    static TokenPtr PLUS_EQUALS();

    static TokenPtr newLine(const ConfigOriginPtr& origin);
    static TokenPtr newProblem(const ConfigOriginPtr& origin,
                               const std::string& what,
                               const std::string& message,
                               bool suggestQuotes);
    static TokenPtr newComment(const ConfigOriginPtr& origin,
                               const std::string& text);
    static TokenPtr newUnquotedText(const ConfigOriginPtr& origin,
                                    const std::string& s);
    static TokenPtr newSubstitution(const ConfigOriginPtr& origin,
                                    bool optional,
                                    const VectorToken& expression);
    static TokenPtr newValue(const AbstractConfigValuePtr& value);
    static TokenPtr newString(const ConfigOriginPtr& origin,
                              const std::string& value);
    static TokenPtr newInt(const ConfigOriginPtr& origin,
                           int32_t value,
                           const std::string& originalText);
    static TokenPtr newDouble(const ConfigOriginPtr& origin,
                              double value,
                              const std::string& originalText);
    static TokenPtr newInt64(const ConfigOriginPtr& origin,
                             int64_t value,
                             const std::string& originalText);
    static TokenPtr newNull(const ConfigOriginPtr& origin);
    static TokenPtr newBoolean(const ConfigOriginPtr& origin,
                               bool value);
};

class ValueToken : public Token {
public:
    CONFIG_CLASS(ValueToken);

    ValueToken(const AbstractConfigValuePtr& value);

    AbstractConfigValuePtr value();

    virtual std::string toString() override;

protected:
    virtual bool canEqual(const ConfigVariant& other) override;

public:
    virtual bool equals(const ConfigVariant& other) override;
    virtual uint32_t hashCode() override;

private:
    AbstractConfigValuePtr value_;
};

class LineToken : public Token {
public:
    CONFIG_CLASS(LineToken);

    LineToken(const ConfigOriginPtr& origin);

    virtual std::string toString() override;

protected:
    virtual bool canEqual(const ConfigVariant& other) override;

public:
    virtual bool equals(const ConfigVariant& other) override;
    virtual uint32_t hashCode() override;
};

///
/// This is not a ValueToken, because it requires special processing
///
class UnquotedTextToken : public Token {
public:
    CONFIG_CLASS(UnquotedTextToken);

    UnquotedTextToken(const ConfigOriginPtr& origin, const std::string& s);

    std::string value();

    virtual std::string toString() override;

protected:
    virtual bool canEqual(const ConfigVariant& other) override;

public:
    virtual bool equals(const ConfigVariant& other) override;
    virtual uint32_t hashCode() override;

private:
    std::string value_;
};

class ProblemToken : public Token {
public:
    CONFIG_CLASS(ProblemToken);

    ProblemToken(const ConfigOriginPtr& origin,
                 const std::string& what,
                 const std::string& message,
                 bool suggestQuotes);

    std::string what();
    std::string message();
    bool suggestQuotes();

    virtual std::string toString() override;

protected:
    virtual bool canEqual(const ConfigVariant& other) override;

public:
    virtual bool equals(const ConfigVariant& other) override;
    virtual uint32_t hashCode() override;

private:
    std::string what_;
    std::string message_;
    bool suggestQuotes_;
};

class CommentToken : public Token {
public:
    CONFIG_CLASS(CommentToken);

    CommentToken(const ConfigOriginPtr& origin, const std::string& text);

    std::string text();

    virtual std::string toString() override;

protected:
    virtual bool canEqual(const ConfigVariant& other) override;

public:
    virtual bool equals(const ConfigVariant& other) override;
    virtual uint32_t hashCode() override;

private:
    std::string text_;
};

///
/// This is not a ValueToken, because it requires special processing
///
class SubstitutionToken : public Token {
public:
    CONFIG_CLASS(SubstitutionToken);

    SubstitutionToken(const ConfigOriginPtr& origin,
                      bool optional,
                      const VectorToken& expression);

    bool optional();
    VectorToken value();

    virtual std::string toString() override;

protected:
    virtual bool canEqual(const ConfigVariant& other) override;

public:
    virtual bool equals(const ConfigVariant& other) override;
    virtual uint32_t hashCode() override;

private:
    bool optional_;
    VectorToken value_;
};

}

#endif // TOKENS_H_
