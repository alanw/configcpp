/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/tokenizer.h"
#include "configcpp/detail/token.h"
#include "configcpp/detail/tokens.h"
#include "configcpp/detail/simple_config_origin.h"
#include "configcpp/detail/reader.h"
#include "configcpp/config_syntax.h"

namespace config {

std::string Tokenizer::asString(int32_t codepoint) {
    if (static_cast<char>(codepoint) == '\n') {
        return "newline";
    }
    else if (static_cast<char>(codepoint) == '\t') {
        return "tab";
    }
    else if (codepoint == -1) {
        return "end of file";
    }
    else if (std::iscntrl(static_cast<char>(codepoint))) {
        std::ostringstream stream;
        stream << "control character 0x" << std::hex << std::nouppercase;
        stream << std::setfill('0') << std::setw(2) << codepoint;
        return stream.str();
    }
    else {
        return std::string(1, static_cast<char>(codepoint));
    }
}

TokenIteratorPtr Tokenizer::tokenize(const ConfigOriginPtr& origin, const ReaderPtr& input, ConfigSyntax flavor) {
    return TokenStream::make_instance(origin, input, flavor != ConfigSyntax::JSON);
}

WhitespaceSaver::WhitespaceSaver() :
    lastTokenWasSimpleValue(false) {
}

void WhitespaceSaver::add(int32_t c) {
    if (lastTokenWasSimpleValue) {
        whitespace << static_cast<char>(c);
    }
}

TokenPtr WhitespaceSaver::check(const TokenPtr& t, const ConfigOriginPtr& baseOrigin, int32_t lineNumber) {
    if (TokenStream::isSimpleValue(t)) {
        return nextIsASimpleValue(baseOrigin, lineNumber);
    }
    else {
        nextIsNotASimpleValue();
        return nullptr;
    }
}

void WhitespaceSaver::nextIsNotASimpleValue() {
    lastTokenWasSimpleValue = false;
    whitespace.str("");
}

TokenPtr WhitespaceSaver::nextIsASimpleValue(const ConfigOriginPtr& baseOrigin, int32_t lineNumber) {
    if (lastTokenWasSimpleValue) {
        // need to save whitespace between the two so
        // the parser has the option to concatenate it.
        if (!whitespace.str().empty()) {
            auto t = Tokens::newUnquotedText(TokenStream::lineOrigin(baseOrigin, lineNumber), whitespace.str());
            whitespace.str(""); // reset
            return t;
        }
        else {
            // lastTokenWasSimpleValue = true still
            return nullptr;
        }
    }
    else {
        lastTokenWasSimpleValue = true;
        whitespace.str("");
        return nullptr;
    }
}

TokenIterator::TokenIterator(const VectorToken& tokens) :
    begin(tokens.begin()),
    end(tokens.end()) {
}

bool TokenIterator::hasNext() {
    return begin != end;
}

TokenPtr TokenIterator::next() {
    return *begin++;
}

const std::string TokenStream::firstNumberChars = "0123456789-";
const std::string TokenStream::numberChars = "0123456789eE+-.";
const std::string TokenStream::notInUnquotedText = "$\"{}[]:=,+#`^?!@*&\\";

TokenStream::TokenStream(const ConfigOriginPtr& origin, const ReaderPtr& input, bool allowComments) :
    origin_(std::dynamic_pointer_cast<SimpleConfigOrigin>(origin)),
    input(input),
    allowComments(allowComments),
    lineNumber(1),
    lineOrigin_(origin_->setLineNumber(lineNumber)),
    tokens(1, Tokens::START()),
    whitespaceSaver(WhitespaceSaver::make_instance()) {
}

int32_t TokenStream::nextCharRaw() {
    if (buffer.empty()) {
        try {
            return input->read();
        }
        catch (ConfigExceptionIO& e) {
            throw ConfigExceptionIO(origin_, std::string("read error: ") + e.what());
        }
    }
    else {
        int32_t c = buffer.front();
        buffer.pop_front();
        return c;
    }
}

void TokenStream::putBack(int32_t c) {
    if (buffer.size() > 2) {
        throw ConfigExceptionBugOrBroken("bug: putBack() three times, undesirable look-ahead");
    }
    buffer.push_front(c);
}

bool TokenStream::isWhitespace(int32_t c) {
    return std::isspace(c);
}

bool TokenStream::isWhitespaceNotNewline(int32_t c) {
    return c != '\n' && std::isspace(c);
}

bool TokenStream::startOfComment(int32_t c) {
    if (c == -1) {
        return false;
    }
    else {
        if (allowComments) {
            if (c == '#') {
                return true;
            }
            else if (c == '/') {
                int32_t maybeSecondSlash = nextCharRaw();
                // we want to predictably NOT consume any chars
                putBack(maybeSecondSlash);
                if (maybeSecondSlash == '/') {
                    return true;
                }
                else {
                    return false;
                }
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }
    }
}

int32_t TokenStream::nextCharAfterWhitespace(const WhitespaceSaverPtr& saver) {
    while (true) {
        int32_t c = nextCharRaw();

        if (c == -1) {
            return -1;
        }
        else {
            if (isWhitespaceNotNewline(c)) {
                saver->add(c);
                continue;
            }
            else {
                return c;
            }
        }
    }
}

ConfigExceptionTokenizerProblem TokenStream::problem(const std::string& message) {
    return problem(lineOrigin_, "", message, false);
}

ConfigExceptionTokenizerProblem TokenStream::problem(const std::string& what, const std::string& message, bool suggestQuotes) {
    return problem(lineOrigin_, what, message, suggestQuotes);
}

ConfigExceptionTokenizerProblem TokenStream::problem(const ConfigOriginPtr& origin, const std::string& what, const std::string& message, bool suggestQuotes) {
    return ConfigExceptionTokenizerProblem(Tokens::newProblem(origin, what, message, suggestQuotes));
}

ConfigExceptionTokenizerProblem TokenStream::problem(const ConfigOriginPtr& origin, const std::string& message) {
    return problem(origin, "", message, false);
}

ConfigOriginPtr TokenStream::lineOrigin(const ConfigOriginPtr& baseOrigin, int32_t lineNumber) {
    return std::dynamic_pointer_cast<SimpleConfigOrigin>(baseOrigin)->setLineNumber(lineNumber);
}

TokenPtr TokenStream::pullComment(int32_t firstChar) {
    if (firstChar == '/') {
        int32_t discard = nextCharRaw();
        if (discard != '/') {
            throw ConfigExceptionBugOrBroken("called pullComment but // not seen");
        }
    }

    std::string s;
    while (true) {
        int32_t c = nextCharRaw();
        if (c == -1 || c == '\n') {
            putBack(c);
            return Tokens::newComment(lineOrigin_, s);
        }
        else {
            s += static_cast<char>(c);
        }
    }
}

TokenPtr TokenStream::pullUnquotedText() {
    auto origin = lineOrigin_;
    std::string sb;
    int32_t c = nextCharRaw();
    while (true) {
        if (c == -1) {
            break;
        }
        else if (notInUnquotedText.find(c) != std::string::npos) {
            break;
        }
        else if (isWhitespace(c)) {
            break;
        }
        else if (startOfComment(c)) {
            break;
        }
        else {
            sb += static_cast<char>(c);
        }

        // we parse true/false/null tokens as such no matter
        // what is after them, as long as they are at the
        // start of the unquoted token.
        if (sb.length() == 4) {
            if (sb == "true") {
                return Tokens::newBoolean(origin, true);
            }
            else if (sb == "null") {
                return Tokens::newNull(origin);
            }
        }
        else if (sb.length() == 5) {
            if (sb == "false") {
                return Tokens::newBoolean(origin, false);
            }
        }

        c = nextCharRaw();
    }

    // put back the char that ended the unquoted text
    putBack(c);

    return Tokens::newUnquotedText(origin, sb);
}

TokenPtr TokenStream::pullNumber(int32_t firstChar) {
    std::string s;
    s += static_cast<char>(firstChar);
    bool containedDecimalOrE = false;
    int32_t c = nextCharRaw();
    while (c != -1 && numberChars.find(c) != std::string::npos) {
        if (c == '.' || c == 'e' || c == 'E') {
            containedDecimalOrE = true;
        }
        s += static_cast<char>(c);
        c = nextCharRaw();
    }
    // the last character we looked at wasn't part of the number, put it back
    putBack(c);
    try {
        if (containedDecimalOrE) {
            // force floating point representation
            return Tokens::newDouble(lineOrigin_, boost::lexical_cast<double>(s), s);
        }
        else {
            // this should throw if the integer is too large for int64_t
            return Tokens::newInt64(lineOrigin_, boost::lexical_cast<int64_t>(s), s);
        }
    }
    catch (boost::bad_lexical_cast&) {
        throw problem(s, "Invalid number: '" + s + "'", true);
    }
}

void TokenStream::pullEscapeSequence(std::string& s) {
    int32_t escaped = nextCharRaw();
    if (escaped == -1) {
        throw problem("End of input but backslash in string had nothing after it");
    }
    switch (escaped) {
        case '"':
            s += '"';
            break;
        case '\\':
            s += '\\';
            break;
        case '/':
            s += '/';
            break;
        case 'b':
            s += '\b';
            break;
        case 'f':
            s += '\f';
            break;
        case 'n':
            s += '\n';
            break;
        case 'r':
            s += '\r';
            break;
        case 't':
            s += '\t';
            break;
        case 'u': {
                // kind of absurdly slow, but screw it for now
                std::string digits;
                for (int32_t i = 0; i < 4; ++i) {
                    int32_t c = nextCharRaw();
                    if (c == -1) {
                        throw problem("End of input but expecting 4 hex digits for \\uXXXX escape");
                    }
                    digits.push_back(static_cast<char>(c));
                }
                s += strtol(digits.c_str(), 0, 16);
            }
            break;
        default:
            throw problem(Tokenizer::asString(escaped),
                          "backslash followed by '" + Tokenizer::asString(escaped) +
                          "', this is not a valid escape sequence (quoted"
                          " strings use JSON escaping, so use"
                          " double-backslash \\\\ for literal backslash)");
    }
}

TokenPtr TokenStream::pullQuotedString() {
    // the open quote has already been consumed
    std::string s;
    int32_t c = static_cast<int32_t>('\0'); // value doesn't get used
    do {
        c = nextCharRaw();
        if (c == -1) {
            throw problem("End of input but string quote was still open");
        }
        if (c == '\\') {
            pullEscapeSequence(s);
        }
        else if (c == '"') {
            // end the loop, done!
        }
        else if (std::iscntrl(static_cast<char>(c))) {
            throw problem(Tokenizer::asString(c), "JSON does not allow unescaped " +
                          Tokenizer::asString(c) + " in quoted strings, use a backslash escape");
        }
        else {
            s += static_cast<char>(c);
        }
    } while (c != '"');
    return Tokens::newString(lineOrigin_, s);
}

TokenPtr TokenStream::pullPlusEquals() {
    // the initial '+' has already been consumed
    int32_t c = nextCharRaw();
    if (c != '=') {
        throw problem(Tokenizer::asString(c), "'+' not followed by =, '" +
                      Tokenizer::asString(c) + "' not allowed after '+'", true);
    }
    return Tokens::PLUS_EQUALS();
}

TokenPtr TokenStream::pullSubstitution() {
    // the initial '$' has already been consumed
    auto origin = lineOrigin_;
    int32_t c = nextCharRaw();
    if (c != '{') {
        throw problem(Tokenizer::asString(c), "'$' not followed by {, '" +
                      Tokenizer::asString(c) + "' not allowed after '$'", true);
    }

    bool optional = false;
    c = nextCharRaw();
    if (c == '?') {
        optional = true;
    }
    else {
        putBack(c);
    }

    auto saver = WhitespaceSaver::make_instance();
    VectorToken expression;

    TokenPtr t;
    do {
        t = pullNextToken(saver);

        // note that we avoid validating the allowed tokens inside
        // the substitution here; we even allow nested substitutions
        // in the tokenizer. The parser sorts it out.
        if (t == Tokens::CLOSE_CURLY()) {
            // end the loop, done!
            break;
        }
        else if (t == Tokens::END()) {
            throw problem(origin, "Substitution ${ was not closed with a }");
        }
        else {
            auto whitespace = saver->check(t, origin, lineNumber);
            if (whitespace) {
                expression.push_back(whitespace);
            }
            expression.push_back(t);
        }
    } while (true);

    return Tokens::newSubstitution(origin, optional, expression);
}

TokenPtr TokenStream::pullNextToken(const WhitespaceSaverPtr& saver) {
    int32_t c = nextCharAfterWhitespace(saver);
    if (c == -1) {
        return Tokens::END();
    }
    else if (c == '\n') {
        // newline tokens have the just-ended line number
        auto line = Tokens::newLine(lineOrigin_);
        lineNumber += 1;
        lineOrigin_ = origin_->setLineNumber(lineNumber);
        return line;
    }
    else {
        TokenPtr t;
        if (startOfComment(c)) {
            t = pullComment(c);
        }
        else {
            switch (c) {
                case '"':
                    t = pullQuotedString();
                    break;
                case '$':
                    t = pullSubstitution();
                    break;
                case ':':
                    t = Tokens::COLON();
                    break;
                case ',':
                    t = Tokens::COMMA();
                    break;
                case '=':
                    t = Tokens::EQUALS();
                    break;
                case '{':
                    t = Tokens::OPEN_CURLY();
                    break;
                case '}':
                    t = Tokens::CLOSE_CURLY();
                    break;
                case '[':
                    t = Tokens::OPEN_SQUARE();
                    break;
                case ']':
                    t = Tokens::CLOSE_SQUARE();
                    break;
                case '+':
                    t = pullPlusEquals();
                    break;
                default:
                    t = nullptr;
                    break;
            }

            if (!t) {
                if (firstNumberChars.find(c) != std::string::npos) {
                    t = pullNumber(c);
                }
                else if (notInUnquotedText.find(c) != std::string::npos) {
                    throw problem(Tokenizer::asString(c), "Reserved character '" +
                                  Tokenizer::asString(c) +
                                  "' is not allowed outside quotes", true);
                }
                else {
                    putBack(c);
                    t = pullUnquotedText();
                }
            }
        }

        if (!t) {
            throw ConfigExceptionBugOrBroken("bug: failed to generate next token");
        }
        return t;
    }
}

bool TokenStream::isSimpleValue(const TokenPtr& t) {
    return Tokens::isSubstitution(t) || Tokens::isUnquotedText(t) || Tokens::isValue(t);
}

void TokenStream::queueNextToken() {
    auto t = pullNextToken(whitespaceSaver);
    auto whitespace = whitespaceSaver->check(t, origin_, lineNumber);
    if (whitespace) {
        tokens.push_back(whitespace);
    }
    tokens.push_back(t);
}

bool TokenStream::hasNext() {
    return !tokens.empty();
}

TokenPtr TokenStream::next() {
    auto t = tokens.front();
    tokens.pop_front();
    if (tokens.empty() && t != Tokens::END()) {
        try {
            queueNextToken();
        }
        catch (ConfigExceptionTokenizerProblem& e) {
            tokens.push_back(e.problem());
        }
        if (tokens.empty()) {
            throw ConfigExceptionBugOrBroken("bug: tokens queue should not be empty here");
        }
    }
    return t;
}

}
