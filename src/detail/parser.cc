/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/parser.h"
#include "configcpp/detail/abstract_config_object.h"
#include "configcpp/detail/simple_includer.h"
#include "configcpp/detail/simple_config_origin.h"
#include "configcpp/detail/simple_config_object.h"
#include "configcpp/detail/simple_config_list.h"
#include "configcpp/detail/tokens.h"
#include "configcpp/detail/token.h"
#include "configcpp/detail/tokenizer.h"
#include "configcpp/detail/config_string.h"
#include "configcpp/detail/config_reference.h"
#include "configcpp/detail/config_concatenation.h"
#include "configcpp/detail/path.h"
#include "configcpp/detail/path_builder.h"
#include "configcpp/detail/substitution_expression.h"
#include "configcpp/detail/string_reader.h"
#include "configcpp/config_parse_options.h"
#include "configcpp/config_syntax.h"
#include "configcpp/config_value_type.h"

namespace config {

AbstractConfigValuePtr Parser::parse(const TokenIteratorPtr& tokens, const ConfigOriginPtr& origin, const ConfigParseOptionsPtr& options, const ConfigIncludeContextPtr& includeContext) {
    auto context = ParseContext::make_instance(options->getSyntax(), origin, tokens, SimpleIncluder::makeFull(options->getIncluder()), includeContext);
    return context->parse();
}

TokenWithComments::TokenWithComments(const TokenPtr& token, const VectorToken& comments) :
    token(token),
    comments(comments) {
}

TokenWithCommentsPtr TokenWithComments::prepend(const VectorToken& earlier) {
    if (comments.empty()) {
        return TokenWithComments::make_instance(token, earlier);
    }
    else {
        VectorToken merged(earlier);
        merged.insert(merged.end(), comments.begin(), comments.end());
        return TokenWithComments::make_instance(token, merged);
    }
}

SimpleConfigOriginPtr TokenWithComments::setComments(const SimpleConfigOriginPtr& origin) {
    if (comments.empty()) {
        return origin;
    }
    else {
        VectorString newComments;
        for (auto& c : comments) {
            newComments.push_back(Tokens::getCommentText(c));
        }
        return origin->setComments(newComments);
    }
}

std::string TokenWithComments::toString() {
    // this ends up in user-visible error messages, so we don't want the comments
    return token->toString();
}

ParseContext::ParseContext(ConfigSyntax flavor, const ConfigOriginPtr& origin, const TokenIteratorPtr& tokens, const FullIncluderPtr& includer, const ConfigIncludeContextPtr& includeContext) :
    lineNumber(1),
    tokens(tokens),
    includer(includer),
    includeContext(includeContext),
    flavor(flavor),
    baseOrigin(origin),
    equalsCount(0) {
}

void ParseContext::consolidateCommentBlock(const TokenPtr& commentToken) {
    // a comment block "goes with" the following token
    // unless it's separated from it by a blank line.
    // we want to build a list of newline tokens followed
    // by a non-newline non-comment token; with all comments
    // associated with that final non-newline non-comment token.
    VectorToken newlines;
    VectorToken comments;

    TokenPtr previous;
    auto next = commentToken;
    while (true) {
        if (Tokens::isNewline(next)) {
            if (previous && Tokens::isNewline(previous)) {
                // blank line; drop all comments to this point and
                // start a new comment block
                comments.clear();
            }
            newlines.push_back(next);
        }
        else if (Tokens::isComment(next)) {
            comments.push_back(next);
        }
        else {
            // a non-newline non-comment token
            break;
        }

        previous = next;
        next = tokens->next();
    }

    // put our concluding token in the queue with all the comments attached
    buffer.push_front(TokenWithComments::make_instance(next, comments));

    // now put all the newlines back in front of it
    for (auto li = newlines.rbegin(); li != newlines.rend(); ++li) {
        buffer.push_front(TokenWithComments::make_instance(*li));
    }
}

TokenWithCommentsPtr ParseContext::popToken() {
    if (buffer.empty()) {
        auto t = tokens->next();
        if (Tokens::isComment(t)) {
            consolidateCommentBlock(t);
            auto token = buffer.front();
            buffer.pop_front();
            return token;
        }
        else {
            return TokenWithComments::make_instance(t);
        }
    }
    else {
        auto token = buffer.front();
        buffer.pop_front();
        return token;
    }
}

TokenWithCommentsPtr ParseContext::nextToken() {
    auto withComments = popToken();
    auto t = withComments->token;

    if (Tokens::isProblem(t)) {
        auto origin = t->origin();
        std::string message = Tokens::getProblemMessage(t);
        bool suggestQuotes = Tokens::getProblemSuggestQuotes(t);
        if (suggestQuotes) {
            message = addQuoteSuggestion(t->toString(), message);
        }
        else {
            message = addKeyName(message);
        }
        throw ConfigExceptionParse(origin, message);
    }
    else {
        if (flavor == ConfigSyntax::JSON) {
            if (Tokens::isUnquotedText(t)) {
                throw parseError(addKeyName("Token not allowed in valid JSON: '" +
                                 Tokens::getUnquotedText(t) + "'"));
            }
            else if (Tokens::isSubstitution(t)) {
                throw parseError(addKeyName("Substitutions (${} syntax) not allowed in JSON"));
            }
        }

        return withComments;
    }
}

void ParseContext::putBack(const TokenWithCommentsPtr& token) {
    buffer.push_front(token);
}

TokenWithCommentsPtr ParseContext::nextTokenIgnoringNewline() {
    auto t = nextToken();

    while (Tokens::isNewline(t->token)) {
        // line number tokens have the line that was _ended_ by the
        // newline, so we have to add one.
        lineNumber = t->token->lineNumber() + 1;

        t = nextToken();
    }

    return t;
}

bool ParseContext::checkElementSeparator() {
    if (flavor == ConfigSyntax::JSON) {
        auto t = nextTokenIgnoringNewline();
        if (t->token == Tokens::COMMA()) {
            return true;
        }
        else {
            putBack(t);
            return false;
        }
    }
    else {
        bool sawSeparatorOrNewline = false;
        auto t = nextToken();
        while (true) {
            if (Tokens::isNewline(t->token)) {
                // newline number is the line just ended, so add one
                lineNumber = t->token->lineNumber() + 1;
                sawSeparatorOrNewline = true;

                // we want to continue to also eat
                // a comma if there is one.
            }
            else if (t->token == Tokens::COMMA()) {
                return true;
            }
            else {
                // non-newline-or-comma
                putBack(t);
                return sawSeparatorOrNewline;
            }
            t = nextToken();
        }
    }
}

SubstitutionExpressionPtr ParseContext::tokenToSubstitutionExpression(const TokenPtr& valueToken) {
    auto expression = Tokens::getSubstitutionPathExpression(valueToken);
    auto path = Parser::parsePathExpression(TokenIterator::make_instance(expression), valueToken->origin());
    bool optional = Tokens::getSubstitutionOptional(valueToken);
    return SubstitutionExpression::make_instance(path, optional);
}

void ParseContext::consolidateValueTokens() {
    // this trick is not done in JSON
    if (flavor == ConfigSyntax::JSON)
        return;

    // create only if we have value tokens
    boost::optional<VectorAbstractConfigValue> values;
    TokenWithCommentsPtr firstValueWithComments;
    // ignore a newline up front
    auto t = nextTokenIgnoringNewline();
    while (true) {
        AbstractConfigValuePtr v;
        if (Tokens::isValue(t->token)) {
            // if we consolidateValueTokens() multiple times then
            // this value could be a concatenation, object, array,
            // or substitution already.
            v = Tokens::getValue(t->token);
        }
        else if (Tokens::isUnquotedText(t->token)) {
            v = ConfigString::make_instance(t->token->origin(), Tokens::getUnquotedText(t->token));
        }
        else if (Tokens::isSubstitution(t->token)) {
            v = ConfigReference::make_instance(t->token->origin(), tokenToSubstitutionExpression(t->token));
        }
        else if (t->token == Tokens::OPEN_CURLY() || t->token == Tokens::OPEN_SQUARE()) {
            // there may be newlines _within_ the objects and arrays
            v = parseValue(t);
        }
        else {
            break;
        }

        if (!v) {
            throw ConfigExceptionBugOrBroken("no value");
        }

        if (!values) {
            values = VectorAbstractConfigValue();
            firstValueWithComments = t;
        }
        values->push_back(v);

        t = nextToken(); // but don't consolidate across a newline
    }
    // the last one wasn't a value token
    putBack(t);

    if (!values) {
        return;
    }

    auto consolidated = ConfigConcatenation::concatenate(*values);

    putBack(TokenWithComments::make_instance(Tokens::newValue(consolidated), firstValueWithComments->comments));
}

ConfigOriginPtr ParseContext::lineOrigin() {
    return std::dynamic_pointer_cast<SimpleConfigOrigin>(baseOrigin)->setLineNumber(lineNumber);
}

ConfigExceptionParse ParseContext::parseError(const std::string& message) {
    return ConfigExceptionParse(lineOrigin(), message);
}

std::string ParseContext::previousFieldName(const PathPtr& lastPath) {
    if (lastPath) {
        return lastPath->render();
    }
    else if (pathStack.empty()) {
        return "";
    }
    else {
        return pathStack.front()->render();
    }
}

PathPtr ParseContext::fullCurrentPath() {
    PathPtr full;
    // pathStack has top of stack at front
    for (auto& p : pathStack) {
        if (!full) {
            full = p;
        }
        else {
            full = full->prepend(p);
        }
    }
    return full;
}

std::string ParseContext::previousFieldName() {
    return previousFieldName(nullptr);
}

std::string ParseContext::addKeyName(const std::string& message) {
    std::string previousFieldName_ = previousFieldName();
    if (previousFieldName_.empty()) {
        return "in value for key '" + previousFieldName_ + "': " + message;
    }
    else {
        return message;
    }
}

std::string ParseContext::addQuoteSuggestion(const std::string& badToken, const std::string& message) {
    return addQuoteSuggestion(nullptr, equalsCount > 0, badToken, message);
}

std::string ParseContext::addQuoteSuggestion(const PathPtr& lastPath, bool insideEquals, const std::string& badToken, const std::string& message) {
    std::string previousFieldName_ = previousFieldName(lastPath);

    std::string part;
    if (badToken == Tokens::END()->toString()) {
        // EOF requires special handling for the error to make sense.
        if (!previousFieldName_.empty()) {
            part = message + " (if you intended '" + previousFieldName_ +
                   "' to be part of a value, instead of a key, " +
                   "try adding double quotes around the whole value";
        }
        else {
            return message;
        }
    }
    else {
        if (!previousFieldName_.empty()) {
            part = message + " (if you intended " + badToken +
                   " to be part of the value for '" + previousFieldName_ +
                   "', try enclosing the value in double quotes";
        }
        else {
            part = message + " (if you intended " + badToken +
                   " to be part of a key or string value, "
                   "try enclosing the key or value in double quotes";
        }
    }

    if (insideEquals) {
        return part + ", or you may be able to rename the file .properties rather than .conf)";
    }
    else {
        return part + ")";
    }
}

AbstractConfigValuePtr ParseContext::parseValue(const TokenWithCommentsPtr& t) {
    AbstractConfigValuePtr v;

    if (Tokens::isValue(t->token)) {
        v = Tokens::getValue(t->token);
    }
    else if (t->token == Tokens::OPEN_CURLY()) {
        v = std::static_pointer_cast<AbstractConfigValue>(parseObject(true));
    }
    else if (t->token == Tokens::OPEN_SQUARE()) {
        v = std::static_pointer_cast<AbstractConfigValue>(parseArray());
    }
    else {
        throw parseError(addQuoteSuggestion(t->token->toString(),
                         "Expecting a value but got wrong token: " + t->token->toString()));
    }

    v = v->withOrigin(t->setComments(std::dynamic_pointer_cast<SimpleConfigOrigin>(v->origin())));

    return v;
}

AbstractConfigObjectPtr ParseContext::createValueUnderPath(const PathPtr& path, const AbstractConfigValuePtr& value) {
    // for path foo.bar, we are creating
    // { "foo" : { "bar" : value } }
    VectorString keys;

    std::string key = path->first();
    auto remaining = path->remainder();
    while (!key.empty()) {
        keys.push_back(key);
        if (!remaining) {
            break;
        }
        else {
            key = remaining->first();
            remaining = remaining->remainder();
        }
    }

    // the setComments(VectorString()) is to ensure comments are only
    // on the exact leaf node they apply to.
    // a comment before "foo.bar" applies to the full setting
    // "foo.bar" not also to "foo"
    auto i = keys.rbegin();
    std::string deepest = *i++;
    auto o = SimpleConfigObject::make_instance(std::dynamic_pointer_cast<SimpleConfigOrigin>(value->origin())->setComments({}), MapAbstractConfigValue({{deepest, value}}));
    for (; i != keys.rend(); ++i) {
        o = SimpleConfigObject::make_instance(std::dynamic_pointer_cast<SimpleConfigOrigin>(value->origin())->setComments({}), MapAbstractConfigValue({{*i, o}}));
    }

    return o;
}

PathPtr ParseContext::parseKey(const TokenWithCommentsPtr& token) {
    if (flavor == ConfigSyntax::JSON) {
        if (Tokens::isValueWithType(token->token, ConfigValueType::STRING)) {
            std::string key = Tokens::getValue(token->token)->unwrapped<std::string>();
            return Path::newKey(key);
        }
        else {
            throw parseError(addKeyName("Expecting close brace } or a field name here, got " + token->toString()));
        }
    }
    else {
        VectorToken expression;
        auto t = token;
        while (Tokens::isValue(t->token) || Tokens::isUnquotedText(t->token)) {
            expression.push_back(t->token);
            t = nextToken(); // note: don't cross a newline
        }

        if (expression.empty()) {
            throw parseError(addKeyName("expecting a close brace or a field name here, got " + t->toString()));
        }

        putBack(t); // put back the token we ended with
        return Parser::parsePathExpression(TokenIterator::make_instance(expression), lineOrigin());
    }
}

bool ParseContext::isIncludeKeyword(const TokenPtr& t) {
    return Tokens::isUnquotedText(t) && Tokens::getUnquotedText(t) == "include";
}

bool ParseContext::isUnquotedWhitespace(const TokenPtr& t) {
    if (!Tokens::isUnquotedText(t)) {
        return false;
    }

    std::string s = Tokens::getUnquotedText(t);

    for (auto& c : s) {
        if (!std::isspace(c)) {
            return false;
        }
    }
    return true;
}

void ParseContext::parseInclude(MapAbstractConfigValue& values) {
    auto t = nextTokenIgnoringNewline();
    while (isUnquotedWhitespace(t->token)) {
        t = nextTokenIgnoringNewline();
    }

    AbstractConfigObjectPtr obj;

    // we either have a quoted string or the "file()" syntax
    if (Tokens::isUnquotedText(t->token)) {
        // get foo(
        std::string kind = Tokens::getUnquotedText(t->token);

        if (kind =="file(") {
        }
        else {
            throw parseError("expecting include parameter to be quoted filename, "
                             "or file(). No spaces are allowed before the open "
                             "paren. Not expecting: " + t->toString());
        }

        // skip space inside parens
        t = nextTokenIgnoringNewline();
        while (isUnquotedWhitespace(t->token)) {
            t = nextTokenIgnoringNewline();
        }

        // quoted string
        std::string name;
        if (Tokens::isValueWithType(t->token, ConfigValueType::STRING)) {
            name = Tokens::getValue(t->token)->unwrapped<std::string>();
        }
        else {
            throw parseError("expecting a quoted string inside file(), rather than: " + t->toString());
        }
        // skip space after string, inside parens
        t = nextTokenIgnoringNewline();
        while (isUnquotedWhitespace(t->token)) {
            t = nextTokenIgnoringNewline();
        }

        if (Tokens::isUnquotedText(t->token) && Tokens::getUnquotedText(t->token) == ")") {
            // OK, close paren
        }
        else {
            throw parseError("expecting a close parentheses ')' here, not: " + t->toString());
        }

        if (kind == "file(") {
            obj = std::dynamic_pointer_cast<AbstractConfigObject>(includer->includeFile(includeContext, name));
        }
        else {
            throw ConfigExceptionBugOrBroken("should not be reached");
        }
    }
    else if (Tokens::isValueWithType(t->token, ConfigValueType::STRING)) {
        std::string name = Tokens::getValue(t->token)->unwrapped<std::string>();
        obj = std::dynamic_pointer_cast<AbstractConfigObject>(includer->include(includeContext, name));
    }
    else {
        throw parseError("include keyword is not followed by a quoted string, but by: " + t->toString());
    }

    if (!pathStack.empty()) {
        auto prefix = Path::make_instance(VectorPath(pathStack.begin(), pathStack.end()));
        obj = std::static_pointer_cast<AbstractConfigObject>(obj->relativized(prefix));
    }

    for (auto& pair : *obj) {
        auto v = std::dynamic_pointer_cast<AbstractConfigValue>(pair.second);
        auto existing = values.find(pair.first);
        if (existing != values.end()) {
            values[pair.first] = std::dynamic_pointer_cast<AbstractConfigValue>(v->withFallback(existing->second));
        }
        else {
            values[pair.first] = v;
        }
    }
}

bool ParseContext::isKeyValueSeparatorToken(const TokenPtr& t) {
    if (flavor == ConfigSyntax::JSON) {
        return t == Tokens::COLON();
    }
    else {
        return t == Tokens::COLON() || t == Tokens::EQUALS() || t == Tokens::PLUS_EQUALS();
    }
}

AbstractConfigObjectPtr ParseContext::parseObject(bool hadOpenCurly) {
    // invoked just after the OPEN_CURLY (or START, if !hadOpenCurly)
    MapAbstractConfigValue values;
    auto objectOrigin = lineOrigin();
    bool afterComma = false;
    PathPtr lastPath;
    bool lastInsideEquals = false;

    while (true) {
        auto t = nextTokenIgnoringNewline();
        if (t->token == Tokens::CLOSE_CURLY()) {
            if (flavor == ConfigSyntax::JSON && afterComma) {
                throw parseError(addQuoteSuggestion(t->toString(),
                                 "expecting a field name after a comma, got a close brace } instead"));
            }
            else if (!hadOpenCurly) {
                throw parseError(addQuoteSuggestion(t->toString(), "unbalanced close brace '}' with no open brace"));
            }
            break;
        }
        else if (t->token == Tokens::END() && !hadOpenCurly) {
            putBack(t);
            break;
        }
        else if (flavor != ConfigSyntax::JSON && isIncludeKeyword(t->token)) {
            parseInclude(values);
            afterComma = false;
        }
        else {
            auto keyToken = t;
            auto path = parseKey(keyToken);
            auto afterKey = nextTokenIgnoringNewline();
            bool insideEquals = false;

            // path must be on-stack while we parse the value
            pathStack.push_front(path);

            TokenWithCommentsPtr valueToken;
            AbstractConfigValuePtr newValue;
            if (flavor == ConfigSyntax::CONF && afterKey->token == Tokens::OPEN_CURLY()) {
                // can omit the ':' or '=' before an object value
                valueToken = afterKey;
            }
            else {
                if (!isKeyValueSeparatorToken(afterKey->token)) {
                    throw parseError(addQuoteSuggestion(afterKey->toString(),
                                     "Key '" + path->render() +
                                     "' may not be followed by token: " + afterKey->toString()));
                }

                if (afterKey->token == Tokens::EQUALS()) {
                    insideEquals = true;
                    equalsCount++;
                }

                consolidateValueTokens();
                valueToken = nextTokenIgnoringNewline();
            }

            newValue = parseValue(valueToken->prepend(keyToken->comments));

            if (afterKey->token == Tokens::PLUS_EQUALS()) {
                VectorAbstractConfigValue concat;
                auto previousRef = ConfigReference::make_instance(newValue->origin(), SubstitutionExpression::make_instance(fullCurrentPath(), true));
                auto list = SimpleConfigList::make_instance(newValue->origin(), VectorAbstractConfigValue({newValue}));
                concat.push_back(previousRef);
                concat.push_back(list);
                newValue = ConfigConcatenation::concatenate(concat);
            }

            lastPath = pathStack.front();
            pathStack.pop_front();
            if (insideEquals) {
                equalsCount--;
            }
            lastInsideEquals = insideEquals;

            std::string key = path->first();
            auto remaining = path->remainder();

            if (!remaining) {
                auto existing = values.find(key);
                if (existing != values.end()) {
                    // In strict JSON, dups should be an error; while in
                    // our custom config language, they should be merged
                    // if the value is an object (or substitution that
                    // could become an object).

                    if (flavor == ConfigSyntax::JSON) {
                        throw parseError("JSON does not allow duplicate fields: '" +
                                         key + "' was already seen at " +
                                         existing->second->origin()->description());
                    }
                    else {
                        newValue = std::dynamic_pointer_cast<AbstractConfigValue>(newValue->withFallback(existing->second));
                    }
                }
                values[key] = newValue;
            }
            else {
                if (flavor == ConfigSyntax::JSON) {
                    throw ConfigExceptionBugOrBroken("somehow got multi-element path in JSON mode");
                }

                auto obj = createValueUnderPath(remaining, newValue);
                auto existing = values.find(key);
                if (existing != values.end()) {
                    obj = std::dynamic_pointer_cast<AbstractConfigObject>(obj->withFallback(existing->second));
                }
                values[key] = obj;
            }

            afterComma = false;
        }

        if (checkElementSeparator()) {
            // continue looping
            afterComma = true;
        }
        else {
            t = nextTokenIgnoringNewline();
            if (t->token == Tokens::CLOSE_CURLY()) {
                if (!hadOpenCurly) {
                    throw parseError(addQuoteSuggestion(lastPath, lastInsideEquals,
                                     t->toString(), "unbalanced close brace '}' with no open brace"));
                }
                break;
            }
            else if (hadOpenCurly) {
                throw parseError(addQuoteSuggestion(lastPath, lastInsideEquals,
                                 t->toString(), "Expecting close brace } or a comma, got " + t->toString()));
            }
            else {
                if (t->token == Tokens::END()) {
                    putBack(t);
                    break;
                }
                else {
                    throw parseError(addQuoteSuggestion(lastPath, lastInsideEquals,
                                     t->toString(), "Expecting end of input or a comma, got " + t->toString()));
                }
            }
        }
    }

    return SimpleConfigObject::make_instance(objectOrigin, values);
}

SimpleConfigListPtr ParseContext::parseArray() {
    // invoked just after the OPEN_SQUARE
    auto arrayOrigin = lineOrigin();
    VectorAbstractConfigValue values;

    consolidateValueTokens();

    auto t = nextTokenIgnoringNewline();

    // special-case the first element
    if (t->token == Tokens::CLOSE_SQUARE()) {
        return SimpleConfigList::make_instance(arrayOrigin, VectorAbstractConfigValue());
    }
    else if (Tokens::isValue(t->token) || t->token == Tokens::OPEN_CURLY() || t->token == Tokens::OPEN_SQUARE()) {
        values.push_back(parseValue(t));
    }
    else {
        throw parseError(addKeyName("List should have ] or a first element after the open [, instead had token: " +
                         t->toString() + " (if you want " + t->toString() + " to be part of a string value, then double-quote it)"));
    }

    // now remaining elements
    while (true) {
        // just after a value
        if (checkElementSeparator()) {
            // comma (or newline equivalent) consumed
        }
        else {
            t = nextTokenIgnoringNewline();
            if (t->token == Tokens::CLOSE_SQUARE()) {
                return SimpleConfigList::make_instance(arrayOrigin, values);
            }
            else {
                throw parseError(addKeyName("List should have ended with ] or had a comma, instead had token: " +
                                 t->toString() + " (if you want " + t->toString() + " to be part of a string value, then double-quote it)"));
            }
        }

        // now just after a comma
        consolidateValueTokens();

        t = nextTokenIgnoringNewline();
        if (Tokens::isValue(t->token) || t->token == Tokens::OPEN_CURLY() || t->token == Tokens::OPEN_SQUARE()) {
            values.push_back(parseValue(t));
        }
        else if (flavor != ConfigSyntax::JSON && t->token == Tokens::CLOSE_SQUARE()) {
            // we allow one trailing comma
            putBack(t);
        }
        else {
            throw parseError(addKeyName("List should have had new element after a comma, instead had token: " +
                             t->toString() + " (if you want the comma or " + t->toString() +
                             " to be part of a string value, then double-quote it)"));
        }
    }
}

AbstractConfigValuePtr ParseContext::parse() {
    auto t = nextTokenIgnoringNewline();
    if (t->token == Tokens::START()) {
        // OK
    }
    else {
        throw ConfigExceptionBugOrBroken("token stream did not begin with START, had " + t->toString());
    }

    t = nextTokenIgnoringNewline();
    AbstractConfigValuePtr result;
    if (t->token == Tokens::OPEN_CURLY() || t->token == Tokens::OPEN_SQUARE()) {
        result = parseValue(t);
    }
    else {
        if (flavor == ConfigSyntax::JSON) {
            if (t->token == Tokens::END()) {
                throw parseError("Empty document");
            }
            else {
                throw parseError("Document must have an object or array at root, unexpected token: " + t->toString());
            }
        }
        else {
            // the root object can omit the surrounding braces.
            // this token should be the first field's key, or part
            // of it, so put it back.
            putBack(t);
            result = parseObject(false);
            // in this case we don't try to use commentsStack comments
            // since they would all presumably apply to fields not the
            // root object
        }
    }

    t = nextTokenIgnoringNewline();
    if (t->token == Tokens::END()) {
        return result;
    }
    else {
        throw parseError("Document has trailing tokens after first object or array: " + t->toString());
    }
}

Element::Element(const std::string& initial, bool canBeEmpty) :
    element(initial),
    canBeEmpty(canBeEmpty) {
}

std::string Element::toString() {
    return "Element(" + element + "," + (canBeEmpty ? "true" : "false") + ")";
}

void Parser::addPathText(VectorElement& buf, bool wasQuoted, const std::string& newText) {
    std::string::size_type i = wasQuoted ? std::string::npos : newText.find('.');
    auto current = buf.back();
    if (i == std::string::npos) {
        // add to current path element
        current->element += newText;
        // any empty quoted string means this element can now be empty.
        if (wasQuoted && current->element.empty())
            current->canBeEmpty = true;
    }
    else {
        // "buf" plus up to the period is an element
        current->element += newText.substr(0, i);
        // then start a new element
        buf.push_back(Element::make_instance("", false));
        // recurse to consume remainder of newText
        addPathText(buf, false, newText.substr(i + 1));
    }
}

PathPtr Parser::parsePathExpression(const TokenIteratorPtr& expression, const ConfigOriginPtr& origin, const std::string& originalText) {
    // each builder in "buf" is an element in the path.
    VectorElement buf;
    buf.push_back(Element::make_instance("", false));

    if (!expression->hasNext()) {
        throw ConfigExceptionBadPath(origin, originalText,
                                     "Expecting a field name or path here, but got nothing");
    }

    while (expression->hasNext()) {
        auto t = expression->next();
        if (Tokens::isValueWithType(t, ConfigValueType::STRING)) {
            auto v = Tokens::getValue(t);
            // this is a quoted string; so any periods
            // in here don't count as path separators
            std::string s = v->transformToString();

            addPathText(buf, true, s);
        }
        else if (t == Tokens::END()) {
            // ignore this; when parsing a file, it should not happen
            // since we're parsing a token list rather than the main
            // token iterator, and when parsing a path expression from the
            // API, it's expected to have an END.
        }
        else {
            // any periods outside of a quoted string count as
            // separators
            std::string text;
            if (Tokens::isValue(t)) {
                // appending a number here may add
                // a period, but we _do_ count those as path
                // separators, because we basically want
                // "foo 3.0bar" to parse as a string even
                // though there's a number in it. The fact that
                // we tokenize non-string values is largely an
                // implementation detail.
                auto v = Tokens::getValue(t);
                text = v->transformToString();
            }
            else if (Tokens::isUnquotedText(t)) {
                text = Tokens::getUnquotedText(t);
            }
            else {
                throw ConfigExceptionBadPath(origin, originalText,
                                             "Token not allowed in path expression: " +
                                             t->toString() +
                                             " (you can double-quote this token if you really want it here)");
            }

            addPathText(buf, false, text);
        }
    }

    auto pb = PathBuilder::make_instance();
    for (auto& e : buf) {
        if (e->element.empty() && !e->canBeEmpty) {
            throw ConfigExceptionBadPath(origin, originalText,
                                         "path has a leading, trailing, or two "
                                         "adjacent period '.' (use quoted \"\" "
                                         "empty string if you want an empty element)");
        }
        else {
            pb->appendKey(e->element);
        }
    }

    return pb->result();
}

PathPtr Parser::parsePath(const std::string& path) {
    static auto apiOrigin = SimpleConfigOrigin::newSimple("path parameter");

    auto speculated = speculativeFastParsePath(path);
    if (speculated) {
        return speculated;
    }

    auto reader = StringReader::make_instance(path);

    ConfigExceptionPtr finally;
    PathPtr pathExpression;
    try {
        auto tokens = Tokenizer::tokenize(apiOrigin, reader, ConfigSyntax::CONF);
        tokens->next(); // drop START
        pathExpression = parsePathExpression(tokens, apiOrigin, path);
    }
    catch (ConfigException& e) {
        finally = e.clone();
    }

    reader->close();

    if (finally) {
        finally->raise();
    }

    return pathExpression;
}

bool Parser::hasUnsafeChars(const std::string& s) {
    for (auto& c : s) {
        if (std::isalpha(c) || c == '.') {
            continue;
        }
        else {
            return true;
        }
    }
    return false;
}

void Parser::appendPathString(const PathBuilderPtr& pb, const std::string& s) {
    std::string::size_type splitAt = s.find('.');
    if (splitAt == std::string::npos) {
        pb->appendKey(s);
    }
    else {
        pb->appendKey(s.substr(0, splitAt));
        appendPathString(pb, s.substr(splitAt + 1));
    }
}

PathPtr Parser::speculativeFastParsePath(const std::string& path) {
    std::string s = boost::trim_copy(path);
    if (s.empty()) {
        return nullptr;
    }
    if (hasUnsafeChars(s)) {
        return nullptr;
    }
    if (boost::starts_with(s, ".") || boost::ends_with(s, ".") || boost::contains(s, "..")) {
        return nullptr; // let the full parser throw the error
    }

    auto pb = PathBuilder::make_instance();
    appendPathString(pb, s);
    return pb->result();
}

}
