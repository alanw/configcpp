/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/parseable.h"
#include "configcpp/detail/config_impl.h"
#include "configcpp/detail/simple_includer.h"
#include "configcpp/detail/simple_include_context.h"
#include "configcpp/detail/simple_config_origin.h"
#include "configcpp/detail/simple_config_object.h"
#include "configcpp/detail/abstract_config_object.h"
#include "configcpp/detail/tokenizer.h"
#include "configcpp/detail/parser.h"
#include "configcpp/detail/string_reader.h"
#include "configcpp/detail/file_reader.h"
#include "configcpp/config_syntax.h"
#include "configcpp/config_value.h"
#include "configcpp/config_value_type.h"
#include "configcpp/config_parse_options.h"

namespace config {

StackParseable Parseable::parseStack;

Parseable::Parseable(const ConfigParseOptionsPtr& baseOptions) :
    baseOptions(baseOptions) {
}

void Parseable::initialize() {
    postConstruct(baseOptions);
}

ConfigParseOptionsPtr Parseable::fixupOptions(const ConfigParseOptionsPtr& baseOptions) {
    ConfigSyntax syntax = baseOptions->getSyntax();
    if (syntax == ConfigSyntax::NONE) {
        syntax = guessSyntax();
    }
    if (syntax == ConfigSyntax::NONE) {
        syntax = ConfigSyntax::CONF;
    }
    auto modified = baseOptions->setSyntax(syntax);

    // make sure the app-provided includer falls back to default
    modified = modified->appendIncluder(ConfigImpl::defaultIncluder());
    // make sure the app-provided includer is complete
    modified = modified->setIncluder(SimpleIncluder::makeFull(modified->getIncluder()));

    return modified;
}

void Parseable::postConstruct(const ConfigParseOptionsPtr& baseOptions) {
    this->initialOptions = fixupOptions(baseOptions);

    this->includeContext_ = SimpleIncludeContext::make_instance(shared_from_this());

    std::string originDescription = initialOptions->getOriginDescription();
    if (!originDescription.empty()) {
        initialOrigin = SimpleConfigOrigin::newSimple(originDescription);
    }
    else {
        initialOrigin = createOrigin();
    }
}

void Parseable::trace(const std::string& message) {
    if (ConfigImpl::traceLoadsEnabled()) {
        ConfigImpl::trace(message);
    }
}

ConfigSyntax Parseable::guessSyntax() {
    return ConfigSyntax::NONE;
}

ConfigParseablePtr Parseable::relativeTo(const std::string& filename) {
    return newNotFound(filename, "filename was not found: '" + filename + "'", options());
}

ConfigIncludeContextPtr Parseable::includeContext() {
    return includeContext_;
}

AbstractConfigObjectPtr Parseable::forceParsedToObject(const ConfigValuePtr& value) {
    if (instanceof<AbstractConfigObject>(value)) {
        return std::dynamic_pointer_cast<AbstractConfigObject>(value);
    }
    else {
        throw ConfigExceptionWrongType(value->origin(), "", "object at file root", ConfigValueTypeEnum::name(value->valueType()));
    }
}

ConfigObjectPtr Parseable::parse(const ConfigParseOptionsPtr& baseOptions) {
    if (parseStack.size() >= MAX_INCLUDE_DEPTH) {
        std::ostringstream stream;
        stream << "include statements nested more than " << MAX_INCLUDE_DEPTH;
        stream << " times, you probably have a cycle in your includes. Trace: ";
        for (auto& p : parseStack) {
            stream << p->toString() << ",\n";
        }
        std::string message = stream.str();
        throw ConfigExceptionParse(initialOrigin, message.substr(0, message.length() - 2));
    }

    parseStack.push_front(shared_from_this());
    ConfigExceptionPtr finally;
    ConfigObjectPtr parseObject;
    try {
        parseObject = forceParsedToObject(parseValue(baseOptions));
    }
    catch (ConfigException& e) {
        finally = e.clone();
    }

    parseStack.pop_front();
    if (parseStack.empty()) {
        parseStack.clear();
    }

    if (finally) {
        finally->raise();
    }

    return parseObject;
}

AbstractConfigValuePtr Parseable::parseValue(const ConfigParseOptionsPtr& baseOptions) {
    // note that we are NOT using our "initialOptions",
    // but using the ones from the passed-in options. The idea is that
    // callers can get our original options and then parse with different
    // ones if they want.
    auto options = fixupOptions(baseOptions);

    // passed-in options can override origin
    ConfigOriginPtr origin;
    if (!options->getOriginDescription().empty()) {
        origin = SimpleConfigOrigin::newSimple(options->getOriginDescription());
    }
    else {
        origin = initialOrigin;
    }
    return parseValue(origin, options);
}

AbstractConfigValuePtr Parseable::parseValue(const ConfigOriginPtr& origin, const ConfigParseOptionsPtr& finalOptions) {
    try {
        return rawParseValue(origin, finalOptions);
    }
    catch (ConfigExceptionIO& e) {
        if (finalOptions->getAllowMissing()) {
            return SimpleConfigObject::makeEmptyMissing(origin);
        }
        else {
            throw ConfigExceptionIO(origin, e.what());
        }
    }
}

AbstractConfigValuePtr Parseable::rawParseValue(const ConfigOriginPtr& origin, const ConfigParseOptionsPtr& finalOptions) {
    auto reader_ = reader();
    ConfigExceptionPtr finally;
    AbstractConfigValuePtr parseValue;
    try {
        parseValue = rawParseValue(reader_, origin, finalOptions);
    }
    catch (ConfigException& e) {
        finally = e.clone();
    }

    reader_->close();

    if (finally) {
        finally->raise();
    }

    return parseValue;
}

AbstractConfigValuePtr Parseable::rawParseValue(const ReaderPtr& reader, const ConfigOriginPtr& origin, const ConfigParseOptionsPtr& finalOptions) {
    auto tokens = Tokenizer::tokenize(origin, reader, finalOptions->getSyntax());
    return Parser::parse(tokens, origin, finalOptions, includeContext());
}

ConfigObjectPtr Parseable::parse() {
    return forceParsedToObject(parseValue(options()));
}

AbstractConfigValuePtr Parseable::parseValue() {
    return parseValue(options());
}

ConfigOriginPtr Parseable::origin() {
    return initialOrigin;
}

ConfigParseOptionsPtr Parseable::options() {
    return initialOptions;
}

std::string Parseable::toString() {
    return getClassName();
}

ConfigSyntax Parseable::syntaxFromExtension(const std::string& name) {
    if (boost::ends_with(name, ".json")) {
        return ConfigSyntax::JSON;
    }
    else if (boost::ends_with(name, ".conf")) {
        return ConfigSyntax::CONF;
    }
    else {
        return ConfigSyntax::NONE;
    }
}

std::string Parseable::relativeTo(const std::string& file, const std::string& filename) {
    boost::filesystem::path child(filename);
    if (child.has_root_directory()) {
        return "";
    }

    boost::filesystem::path parent = boost::filesystem::path(file).parent_path();

    if (parent.empty()) {
        return "";
    }
    else {
        parent /= filename;
        return parent.directory_string();
    }
}

ParseableNotFound::ParseableNotFound(const std::string& what, const std::string& message, const ConfigParseOptionsPtr& options) :
    Parseable(options),
    what(what),
    message(message) {
}

ReaderPtr ParseableNotFound::reader() {
    throw ConfigExceptionFileNotFound(message);
}

ConfigOriginPtr ParseableNotFound::createOrigin() {
    return SimpleConfigOrigin::newSimple(what);
}

ParseablePtr Parseable::newNotFound(const std::string& whatNotFound, const std::string& message, const ConfigParseOptionsPtr& options) {
    return ParseableNotFound::make_instance(whatNotFound, message, options);
}

ParseableReader::ParseableReader(const ReaderPtr& reader, const ConfigParseOptionsPtr& options) :
    Parseable(options),
    reader_(reader) {
}

ReaderPtr ParseableReader::reader() {
    if (ConfigImpl::traceLoadsEnabled()) {
        trace("Loading config from reader: " + boost::lexical_cast<std::string>(reader_.get()));
    }
    return reader_;
}

ConfigOriginPtr ParseableReader::createOrigin() {
    return SimpleConfigOrigin::newSimple("reader");
}

ParseablePtr Parseable::newReader(const ReaderPtr& reader, const ConfigParseOptionsPtr& options) {
    return ParseableReader::make_instance(reader, options);
}

ParseableString::ParseableString(const std::string& input, const ConfigParseOptionsPtr& options) :
    Parseable(options),
    input(input) {
}

ReaderPtr ParseableString::reader() {
    if (ConfigImpl::traceLoadsEnabled()) {
        trace("Loading config from a string: " + input);
    }
    return StringReader::make_instance(input);
}

ConfigOriginPtr ParseableString::createOrigin() {
    return SimpleConfigOrigin::newSimple("string");
}

std::string ParseableString::toString() {
    return getClassName() + "(" + input + ")";
}

ParseablePtr Parseable::newString(const std::string& input, const ConfigParseOptionsPtr& options) {
    return ParseableString::make_instance(input, options);
}

ParseableFile::ParseableFile(const std::string& input, const ConfigParseOptionsPtr& options) :
    Parseable(options),
    input(input) {
}

ReaderPtr ParseableFile::reader() {
    if (ConfigImpl::traceLoadsEnabled()) {
        trace("Loading config from a file: " + input);
    }
    return FileReader::make_instance(input);
}

ConfigSyntax ParseableFile::guessSyntax() {
    return syntaxFromExtension(input);
}

ConfigParseablePtr ParseableFile::relativeTo(const std::string& filename) {
    boost::filesystem::path sibling;
    if (boost::filesystem::path(filename).has_root_directory()) {
        sibling = boost::filesystem::path(filename);
    }
    else {
        // this may return empty
        sibling = boost::filesystem::path(relativeTo(input, filename));
    }
    if (sibling.empty()) {
        return nullptr;
    }
    if (MiscUtils::fileExists(sibling.directory_string())) {
        return newFile(sibling.directory_string(), options()->setOriginDescription(""));
    }
    else {
        return Parseable::relativeTo(filename);
    }
}

ConfigOriginPtr ParseableFile::createOrigin() {
    return SimpleConfigOrigin::newFile(input);
}

std::string ParseableFile::toString() {
    return getClassName() + "(" + input + ")";
}

ParseablePtr Parseable::newFile(const std::string& input, const ConfigParseOptionsPtr& options) {
    return ParseableFile::make_instance(input, options);
}

}
