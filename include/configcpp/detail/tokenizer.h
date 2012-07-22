/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef TOKENIZER_H_
#define TOKENIZER_H_

#include "configcpp/detail/config_base.h"
#include "configcpp/config_exception.h"

namespace config {

class Tokenizer : public ConfigBase {
public:
    CONFIG_CLASS(Tokenizer);

public:
    static std::string asString(int32_t codepoint);

    /// Tokenizes a Reader. Does not close the reader; you have to arrange to do
    /// that after you're done with the returned iterator.
    static TokenIteratorPtr tokenize(const ConfigOriginPtr& origin,
                                     const ReaderPtr& input,
                                     ConfigSyntax flavor);
};

class WhitespaceSaver : public ConfigBase {
public:
    CONFIG_CLASS(WhitespaceSaver);

    WhitespaceSaver();

    void add(int32_t c);
    TokenPtr check(const TokenPtr& t,
                   const ConfigOriginPtr& baseOrigin,
                   int32_t lineNumber);

private:
    /// Called if the next token is not a simple value;
    /// discards any whitespace we were saving between
    /// simple values.
    void nextIsNotASimpleValue();

    /// Called if the next token IS a simple value,
    /// so creates a whitespace token if the previous
    /// token also was.
    TokenPtr nextIsASimpleValue(const ConfigOriginPtr& baseOrigin,
                                int32_t lineNumber);

private:
    // has to be saved inside value concatenations
    std::ostringstream whitespace;
    // may need to value-concat with next value
    bool lastTokenWasSimpleValue;
};

class TokenIterator : public ConfigBase {
public:
    CONFIG_CLASS(TokenIterator);

    TokenIterator(const VectorToken& tokens = {});

    virtual bool hasNext();
    virtual TokenPtr next();

private:
    VectorToken::const_iterator begin;
    VectorToken::const_iterator end;
};

class TokenStream : public TokenIterator {
public:
    CONFIG_CLASS(TokenStream);

    TokenStream(const ConfigOriginPtr& origin,
                const ReaderPtr& input,
                bool allowComments);

private:
    int32_t nextCharRaw();
    void putBack(int32_t c);

public:
    static bool isWhitespace(int32_t c);
    static bool isWhitespaceNotNewline(int32_t c);

private:
    bool startOfComment(int32_t c);

    /// Get next char, skipping non-newline whitespace
    int32_t nextCharAfterWhitespace(const WhitespaceSaverPtr& saver);

    ConfigExceptionTokenizerProblem problem(const std::string& message);
    ConfigExceptionTokenizerProblem problem(const std::string& what,
                                            const std::string& message,
                                            bool suggestQuotes = false);

    static ConfigExceptionTokenizerProblem problem(const ConfigOriginPtr& origin,
                                                   const std::string& what,
                                                   const std::string& message,
                                                   bool suggestQuotes = false);
    static ConfigExceptionTokenizerProblem problem(const ConfigOriginPtr& origin,
                                                   const std::string& message);

public:
    static ConfigOriginPtr lineOrigin(const ConfigOriginPtr& baseOrigin, int32_t lineNumber);

private:
    // chars JSON allows a number to start with
    static const std::string firstNumberChars;

    // chars JSON allows to be part of a number
    static const std::string numberChars;

    // chars that stop an unquoted string
    static const std::string notInUnquotedText;

    /// ONE char has always been consumed, either the # or the first /, but
    /// not both slashes
    TokenPtr pullComment(int32_t firstChar);

    TokenPtr pullUnquotedText();

    TokenPtr pullNumber(int32_t firstChar);

    void pullEscapeSequence(std::string& s);

    TokenPtr pullQuotedString();

    TokenPtr pullPlusEquals();

    TokenPtr pullSubstitution();

    TokenPtr pullNextToken(const WhitespaceSaverPtr& saver);

public:
    static bool isSimpleValue(const TokenPtr& t);

private:
    void queueNextToken();

public:
    virtual bool hasNext() override;
    virtual TokenPtr next() override;

private:
    SimpleConfigOriginPtr origin_;
    ReaderPtr input;
    bool allowComments;
    int32_t lineNumber;
    ConfigOriginPtr lineOrigin_;
    QueueToken tokens;
    QueueInt buffer;
    WhitespaceSaverPtr whitespaceSaver;
};

}

#endif // TOKENIZER_H_
