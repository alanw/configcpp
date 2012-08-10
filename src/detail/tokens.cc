/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/tokens.h"
#include "configcpp/detail/token.h"
#include "configcpp/detail/token_type.h"
#include "configcpp/detail/abstract_config_value.h"
#include "configcpp/detail/config_string.h"
#include "configcpp/detail/config_number.h"
#include "configcpp/detail/config_null.h"
#include "configcpp/detail/config_boolean.h"
#include "configcpp/config_value_type.h"

namespace config {

ValueToken::ValueToken(const AbstractConfigValuePtr& value) :
    Token(TokenType::VALUE, value->origin()),
    value_(value) {
}

AbstractConfigValuePtr ValueToken::value() {
    return value_;
}

std::string ValueToken::toString() {
    std::ostringstream stream;
    ConfigVariant u = value()->unwrapped();
    stream << "'" << boost::apply_visitor(VariantString(), u);
    stream << "' (" << ConfigValueTypeEnum::name(value_->valueType()) << ")";
    return stream.str();
}

bool ValueToken::canEqual(const ConfigVariant& other) {
    return instanceof<ValueToken>(other);
}

bool ValueToken::equals(const ConfigVariant& other) {
    return Token::equals(other) && static_get<ValueToken>(other)->value_->equals(value_);
}

uint32_t ValueToken::hashCode() {
    uint32_t valueHash = 0;
    if (value_->valueType() == ConfigValueType::BOOLEAN) {
        valueHash = variant_get<bool>(value_->unwrapped()) ? 1231 : 1237;
    }
    else {
        valueHash = value_->hashCode();
    }
    return 41 * (41 + Token::hashCode()) + valueHash;
}

LineToken::LineToken(const ConfigOriginPtr& origin) :
    Token(TokenType::NEWLINE, origin) {
}

std::string LineToken::toString() {
    return "'\\n'@" + boost::lexical_cast<std::string>(lineNumber());
}

bool LineToken::canEqual(const ConfigVariant& other) {
    return instanceof<LineToken>(other);
}

bool LineToken::equals(const ConfigVariant& other) {
    return Token::equals(other) && static_get<LineToken>(other)->lineNumber() == lineNumber();
}

uint32_t LineToken::hashCode() {
    return 41 * (41 + Token::hashCode()) + lineNumber();
}

UnquotedTextToken::UnquotedTextToken(const ConfigOriginPtr& origin, const std::string& s) :
    Token(TokenType::UNQUOTED_TEXT, origin),
    value_(s) {
}

std::string UnquotedTextToken::value() {
    return value_;
}

std::string UnquotedTextToken::toString() {
    return "'" + value_ + "'";
}

bool UnquotedTextToken::canEqual(const ConfigVariant& other) {
    return instanceof<UnquotedTextToken>(other);
}

bool UnquotedTextToken::equals(const ConfigVariant& other) {
    return Token::equals(other) && static_get<UnquotedTextToken>(other)->value_ == value_;
}

uint32_t UnquotedTextToken::hashCode() {
    return 41 * (41 + Token::hashCode()) + std::hash<std::string>()(value_);
}

ProblemToken::ProblemToken(const ConfigOriginPtr& origin, const std::string& what, const std::string& message, bool suggestQuotes) :
    Token(TokenType::PROBLEM, origin),
    what_(what),
    message_(message),
    suggestQuotes_(suggestQuotes) {
}

std::string ProblemToken::what() {
    return what_;
}

std::string ProblemToken::message() {
    return message_;
}

bool ProblemToken::suggestQuotes() {
    return suggestQuotes_;
}

std::string ProblemToken::toString() {
    return "'" + what_ + "' (" + message_ + ")";
}

bool ProblemToken::canEqual(const ConfigVariant& other) {
    return instanceof<ProblemToken>(other);
}

bool ProblemToken::equals(const ConfigVariant& other) {
    return Token::equals(other) &&
           static_get<ProblemToken>(other)->what_ == what_ &&
           static_get<ProblemToken>(other)->message_ == message_ &&
           static_get<ProblemToken>(other)->suggestQuotes_ == suggestQuotes_;
}

uint32_t ProblemToken::hashCode() {
    uint32_t h = 41 * (41 + Token::hashCode());
    h = 41 * (h + std::hash<std::string>()(what_));
    h = 41 * (h + std::hash<std::string>()(message_));
    h = 41 * (h + std::hash<bool>()(suggestQuotes_));
    return h;
}

CommentToken::CommentToken(const ConfigOriginPtr& origin, const std::string& text) :
    Token(TokenType::COMMENT, origin),
    text_(text) {
}

std::string CommentToken::text() {
    return text_;
}

std::string CommentToken::toString() {
    return "'#" + text_ + "' (COMMENT)";
}

bool CommentToken::canEqual(const ConfigVariant& other) {
    return instanceof<CommentToken>(other);
}

bool CommentToken::equals(const ConfigVariant& other) {
    return Token::equals(other) && static_get<CommentToken>(other)->text_ == text_;
}

uint32_t CommentToken::hashCode() {
    return 41 * (41 + Token::hashCode()) + std::hash<std::string>()(text_);
}

SubstitutionToken::SubstitutionToken(const ConfigOriginPtr& origin, bool optional, const VectorToken& expression) :
    Token(TokenType::SUBSTITUTION, origin),
    optional_(optional),
    value_(expression) {
}

bool SubstitutionToken::optional() {
    return optional_;
}

VectorToken SubstitutionToken::value() {
    return value_;
}

std::string SubstitutionToken::toString() {
    std::ostringstream stream;
    for (auto& t : value_) {
        stream << t->toString();
    }
    return "'${" + stream.str() + "}'";
}

bool SubstitutionToken::canEqual(const ConfigVariant& other) {
    return instanceof<SubstitutionToken>(other);
}

bool SubstitutionToken::equals(const ConfigVariant& other) {
    return Token::equals(other) &&
           this->value_.size() == static_get<SubstitutionToken>(other)->value_.size() &&
           std::equal(this->value_.begin(), this->value_.end(), static_get<SubstitutionToken>(other)->value_.begin(), configEquals<TokenPtr>());
}

uint32_t SubstitutionToken::hashCode() {
    size_t hash = 41 * (41 + Token::hashCode());
    for (auto& t : value_) {
        boost::hash_combine(hash, t->hashCode());
    }
    return static_cast<uint32_t>(hash);
}

bool Tokens::isValue(const TokenPtr& token) {
    return instanceof<ValueToken>(token);
}

AbstractConfigValuePtr Tokens::getValue(const TokenPtr& token) {
    if (instanceof<ValueToken>(token)) {
        return std::static_pointer_cast<ValueToken>(token)->value();
    }
    else {
        throw ConfigExceptionBugOrBroken("tried to get value of non-value token " + token->toString());
    }
}

bool Tokens::isValueWithType(const TokenPtr& t, ConfigValueType valueType) {
    return isValue(t) && getValue(t)->valueType() == valueType;
}

bool Tokens::isNewline(const TokenPtr& token) {
    return instanceof<LineToken>(token);
}

bool Tokens::isProblem(const TokenPtr& token) {
    return instanceof<ProblemToken>(token);
}

std::string Tokens::getProblemWhat(const TokenPtr& token) {
    if (instanceof<ProblemToken>(token)) {
        return std::static_pointer_cast<ProblemToken>(token)->what();
    }
    else {
        throw ConfigExceptionBugOrBroken("tried to get problem what from " + token->toString());
    }
}

std::string Tokens::getProblemMessage(const TokenPtr& token) {
    if (instanceof<ProblemToken>(token)) {
        return std::static_pointer_cast<ProblemToken>(token)->message();
    }
    else {
        throw ConfigExceptionBugOrBroken("tried to get problem message from " + token->toString());
    }
}

bool Tokens::getProblemSuggestQuotes(const TokenPtr& token) {
    if (instanceof<ProblemToken>(token)) {
        return std::static_pointer_cast<ProblemToken>(token)->suggestQuotes();
    }
    else {
        throw ConfigExceptionBugOrBroken("tried to get problem suggestQuotes from " + token->toString());
    }
}

bool Tokens::isComment(const TokenPtr& token) {
    return instanceof<CommentToken>(token);
}

std::string Tokens::getCommentText(const TokenPtr& token) {
    if (instanceof<CommentToken>(token)) {
        return std::static_pointer_cast<CommentToken>(token)->text();
    }
    else {
        throw ConfigExceptionBugOrBroken("tried to get comment text from " + token->toString());
    }
}

bool Tokens::isUnquotedText(const TokenPtr& token) {
    return instanceof<UnquotedTextToken>(token);
}

std::string Tokens::getUnquotedText(const TokenPtr& token) {
    if (instanceof<UnquotedTextToken>(token)) {
        return std::static_pointer_cast<UnquotedTextToken>(token)->value();
    }
    else {
        throw ConfigExceptionBugOrBroken("tried to get unquoted text from " + token->toString());
    }
}

bool Tokens::isSubstitution(const TokenPtr& token) {
    return instanceof<SubstitutionToken>(token);
}

VectorToken Tokens::getSubstitutionPathExpression(const TokenPtr& token) {
    if (instanceof<SubstitutionToken>(token)) {
        return std::static_pointer_cast<SubstitutionToken>(token)->value();
    }
    else {
        throw ConfigExceptionBugOrBroken("tried to get substitution from " + token->toString());
    }
}

bool Tokens::getSubstitutionOptional(const TokenPtr& token) {
    if (instanceof<SubstitutionToken>(token)) {
        return std::static_pointer_cast<SubstitutionToken>(token)->optional();
    }
    else {
        throw ConfigExceptionBugOrBroken("tried to get substitution optionality from " + token->toString());
    }
}

TokenPtr Tokens::START() {
    static auto start = Token::newWithoutOrigin(TokenType::START, "start of file");
    return start;
}

TokenPtr Tokens::END() {
    static auto end = Token::newWithoutOrigin(TokenType::END, "end of file");
    return end;
}

TokenPtr Tokens::COMMA() {
    static auto comma = Token::newWithoutOrigin(TokenType::COMMA, "','");
    return comma;
}

TokenPtr Tokens::EQUALS() {
    static auto equals = Token::newWithoutOrigin(TokenType::EQUALS, "'='");
    return equals;
}

TokenPtr Tokens::COLON() {
    static auto colon = Token::newWithoutOrigin(TokenType::COLON, "':'");
    return colon;
}

TokenPtr Tokens::OPEN_CURLY() {
    static auto openCurly = Token::newWithoutOrigin(TokenType::OPEN_CURLY, "'{'");
    return openCurly;
}

TokenPtr Tokens::CLOSE_CURLY() {
    static auto closeCurly = Token::newWithoutOrigin(TokenType::CLOSE_CURLY, "'}'");
    return closeCurly;
}

TokenPtr Tokens::OPEN_SQUARE() {
    static auto openSquare = Token::newWithoutOrigin(TokenType::OPEN_SQUARE, "'['");
    return openSquare;
}

TokenPtr Tokens::CLOSE_SQUARE() {
    static auto closeSquare = Token::newWithoutOrigin(TokenType::CLOSE_SQUARE, "']'");
    return closeSquare;
}

TokenPtr Tokens::PLUS_EQUALS() {
    static auto plusEquals = Token::newWithoutOrigin(TokenType::PLUS_EQUALS, "'+='");
    return plusEquals;
}

TokenPtr Tokens::newLine(const ConfigOriginPtr& origin) {
    return LineToken::make_instance(origin);
}

TokenPtr Tokens::newProblem(const ConfigOriginPtr& origin, const std::string& what, const std::string& message, bool suggestQuotes) {
    return ProblemToken::make_instance(origin, what, message, suggestQuotes);
}

TokenPtr Tokens::newComment(const ConfigOriginPtr& origin, const std::string& text) {
    return CommentToken::make_instance(origin, text);
}

TokenPtr Tokens::newUnquotedText(const ConfigOriginPtr& origin, const std::string& s) {
    return UnquotedTextToken::make_instance(origin, s);
}

TokenPtr Tokens::newSubstitution(const ConfigOriginPtr& origin, bool optional, const VectorToken& expression) {
    return SubstitutionToken::make_instance(origin, optional, expression);
}

TokenPtr Tokens::newValue(const AbstractConfigValuePtr& value) {
    return ValueToken::make_instance(value);
}

TokenPtr Tokens::newString(const ConfigOriginPtr& origin, const std::string& value) {
    return newValue(ConfigString::make_instance(origin, value));
}

TokenPtr Tokens::newInt(const ConfigOriginPtr& origin, int32_t value, const std::string& originalText) {
    return newValue(ConfigNumber::newNumber(origin, static_cast<int64_t>(value), originalText));
}

TokenPtr Tokens::newDouble(const ConfigOriginPtr& origin, double value, const std::string& originalText) {
    return newValue(ConfigNumber::newNumber(origin, value, originalText));
}

TokenPtr Tokens::newInt64(const ConfigOriginPtr& origin, int64_t value, const std::string& originalText) {
    return newValue(ConfigNumber::newNumber(origin, value, originalText));
}

TokenPtr Tokens::newNull(const ConfigOriginPtr& origin) {
    return newValue(ConfigNull::make_instance(origin));
}

TokenPtr Tokens::newBoolean(const ConfigOriginPtr& origin, bool value) {
    return newValue(ConfigBoolean::make_instance(origin, value));
}

}
