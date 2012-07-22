/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/config_exception.h"
#include "configcpp/config_origin.h"

namespace config {

ConfigException::ConfigException() :
    ConfigException("") {
}

ConfigException::ConfigException(const ConfigOriginPtr& origin, const std::string& message) :
    std::runtime_error(origin->description() + ": " + message),
    origin_(origin) {
}

ConfigException::ConfigException(const std::string& message) :
    std::runtime_error(message) {
}

ConfigOriginPtr ConfigException::origin() {
    return origin_;
}

ConfigExceptionWrongType::ConfigExceptionWrongType(const ConfigOriginPtr& origin, const std::string& path, const std::string& expected, const std::string& actual) :
    ConfigException(origin, path + " has type " + actual + " rather than " + expected) {
}

ConfigExceptionWrongType::ConfigExceptionWrongType(const ConfigOriginPtr& origin, const std::string& message) :
    ConfigException(origin, message) {
}

ConfigExceptionMissing::ConfigExceptionMissing(const std::string& path) :
    ConfigException("No configuration setting found for key '" + path + "'") {
}

ConfigExceptionMissing::ConfigExceptionMissing(const ConfigOriginPtr& origin, const std::string& message) :
    ConfigException(origin, message) {
}

ConfigExceptionNull::ConfigExceptionNull(const ConfigOriginPtr& origin, const std::string& path, const std::string& expected) :
    ConfigExceptionMissing(origin, makeMessage(path, expected)) {
}

std::string ConfigExceptionNull::makeMessage(const std::string& path, const std::string& expected) {
    if (!expected.empty()) {
        return "Configuration key '" + path + "' is set to null but expected " + expected;
    }
    else {
        return "Configuration key '" + path + "' is null";
    }
}

ConfigExceptionBadValue::ConfigExceptionBadValue(const ConfigOriginPtr& origin, const std::string& path, const std::string& message) :
    ConfigException(origin, "Invalid value at '" + path + "': " + message) {
}

ConfigExceptionBadValue::ConfigExceptionBadValue(const std::string& path, const std::string& message) :
    ConfigException("Invalid value at '" + path + "': " + message) {
}

ConfigExceptionBadPath::ConfigExceptionBadPath(const ConfigOriginPtr& origin, const std::string& path, const std::string& message) :
    ConfigException(origin, !path.empty() ? ("Invalid path '" + path + "': " + message) : message) {
}

ConfigExceptionBadPath::ConfigExceptionBadPath(const std::string& path, const std::string& message) :
    ConfigException(!path.empty() ? ("Invalid path '" + path + "': " + message) : message) {
}

ConfigExceptionBadPath::ConfigExceptionBadPath(const ConfigOriginPtr& origin, const std::string& message) :
    ConfigExceptionBadPath(origin, "", message) {
}

ConfigExceptionBugOrBroken::ConfigExceptionBugOrBroken(const std::string& message) :
    ConfigException(message) {
}

ConfigExceptionIO::ConfigExceptionIO(const ConfigOriginPtr& origin, const std::string& message) :
    ConfigException(origin, message) {
}

ConfigExceptionIO::ConfigExceptionIO(const std::string& message) :
    ConfigException(message) {
}

ConfigExceptionParse::ConfigExceptionParse(const ConfigOriginPtr& origin, const std::string& message) :
    ConfigException(origin, message) {
}

ConfigExceptionUnresolvedSubstitution::ConfigExceptionUnresolvedSubstitution(const ConfigOriginPtr& origin, const std::string& detail) :
    ConfigExceptionParse(origin, "Could not resolve substitution to a value: " + detail) {
}

ConfigExceptionNotResolved::ConfigExceptionNotResolved(const std::string& message) :
    ConfigExceptionBugOrBroken(message) {
}

ValidationProblem::ValidationProblem(const std::string& path, const ConfigOriginPtr& origin, const std::string& problem) :
    path_(path),
    origin_(origin),
    problem_(problem) {
}

std::string ValidationProblem::path() {
    return path_;
}

ConfigOriginPtr ValidationProblem::origin() {
    return origin_;
}

std::string ValidationProblem::problem() {
    return problem_;
}

ConfigExceptionValidationFailed::ConfigExceptionValidationFailed(const VectorValidationProblem& problems) :
    ConfigException(makeMessage(problems)),
    problems_(problems) {
}

const VectorValidationProblem& ConfigExceptionValidationFailed::problems() {
    return problems_;}

std::string ConfigExceptionValidationFailed::makeMessage(const VectorValidationProblem& problems) {
    std::ostringstream stream;
    for (auto p : problems) {
        stream << p.origin()->description();
        stream << ": ";
        stream << p.path();
        stream << ": ";
        stream << p.problem();
        stream << ", ";
    }
    if (problems.empty()) {
        throw ConfigExceptionBugOrBroken("ValidationFailed must have a non-empty list of problems");
    }
    std::string message = stream.str();
    return message.substr(0, message.length() - 2); // chop comma and space
}

ConfigExceptionUnsupportedOperation::ConfigExceptionUnsupportedOperation(const std::string& message) :
    ConfigException(message) {
}

ConfigExceptionFileNotFound::ConfigExceptionFileNotFound(const std::string& file) :
    ConfigExceptionIO(file + " not found") {
}

ConfigExceptionTokenizerProblem::ConfigExceptionTokenizerProblem(const TokenPtr& problem) :
    ConfigException(problem->toString()),
    problem_(problem) {
}

TokenPtr ConfigExceptionTokenizerProblem::problem() {
    return problem_;
}

ConfigExceptionGeneric::ConfigExceptionGeneric(const std::string& message) :
    ConfigException(message) {
}

}
