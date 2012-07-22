/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_EXCEPTION_H_
#define CONFIG_EXCEPTION_H_

#include "configcpp/detail/token.h"

namespace config {

#define EXCEPTION_CLASS(Name) \
    virtual void raise() { \
        throw *this; \
    } \
    virtual ConfigExceptionPtr clone() { \
        return std::make_shared<Name>(*this); \
    }

///
/// All exceptions thrown by the library are subclasses of
/// <code>ConfigException</code>.
///
class ConfigException : public std::runtime_error {
public:
    EXCEPTION_CLASS(ConfigException)

    ConfigException();

protected:
    ConfigException(const ConfigOriginPtr& origin, const std::string& message = "");
    ConfigException(const std::string& message);

public:
    /// Returns an "origin" (such as a filename and line number) for the
    /// exception, or null if none is available. If there's no sensible origin
    /// for a given exception, or the kind of exception doesn't meaningfully
    /// relate to a particular origin file, this returns null. Never assume this
    /// will return non-null, it can always return null.
    ///
    /// @return origin of the problem, or null if unknown / inapplicable
    ConfigOriginPtr origin();

private:
    ConfigOriginPtr origin_;
};

///
/// Exception indicating that the type of a value does not match the type you
/// requested.
///
class ConfigExceptionWrongType : public ConfigException {
public:
    EXCEPTION_CLASS(ConfigExceptionWrongType)

    ConfigExceptionWrongType(const ConfigOriginPtr& origin,
                             const std::string& path,
                             const std::string& expected,
                             const std::string& actual);
    ConfigExceptionWrongType(const ConfigOriginPtr& origin,
                             const std::string& message);
};

///
/// Exception indicates that the setting was never set to anything, not even
/// null.
///
class ConfigExceptionMissing : public ConfigException {
public:
    EXCEPTION_CLASS(ConfigExceptionMissing)

    ConfigExceptionMissing(const std::string& path);

protected:
    ConfigExceptionMissing(const ConfigOriginPtr& origin,
                           const std::string& message);
};

///
/// Exception indicates that the setting was treated as missing because it
/// was set to null.
///
class ConfigExceptionNull : public ConfigExceptionMissing {
public:
    EXCEPTION_CLASS(ConfigExceptionNull)

    ConfigExceptionNull(const ConfigOriginPtr& origin,
                        const std::string& path,
                        const std::string& expected);

private:
    static std::string makeMessage(const std::string& path,
                                   const std::string& expected);
};

///
/// Exception indicating that a value was messed up, for example you may have
/// asked for a duration and the value can't be sensibly parsed as a
/// duration.
///
class ConfigExceptionBadValue : public ConfigException {
public:
    EXCEPTION_CLASS(ConfigExceptionBadValue)

    ConfigExceptionBadValue(const ConfigOriginPtr& origin,
                            const std::string& path,
                            const std::string& message);
    ConfigExceptionBadValue(const std::string& path,
                            const std::string& message);
};

///
/// Exception indicating that a path expression was invalid. Try putting
/// double quotes around path elements that contain "special" characters.
///
class ConfigExceptionBadPath : public ConfigException {
public:
    EXCEPTION_CLASS(ConfigExceptionBadPath)

    ConfigExceptionBadPath(const ConfigOriginPtr& origin,
                           const std::string& path,
                           const std::string& message);
    ConfigExceptionBadPath(const std::string& path,
                           const std::string& message);
    ConfigExceptionBadPath(const ConfigOriginPtr& origin,
                           const std::string& message);
};

///
/// Exception indicating that there's a bug in something (possibly the
/// library itself) or the runtime environment is broken. This exception
/// should never be handled; instead, something should be fixed to keep the
/// exception from occurring. This exception can be thrown by any method in
/// the library.
///
class ConfigExceptionBugOrBroken : public ConfigException {
public:
    EXCEPTION_CLASS(ConfigExceptionBugOrBroken)

    ConfigExceptionBugOrBroken(const std::string& message);
};

///
/// Exception indicating that there was an IO error.
///
class ConfigExceptionIO : public ConfigException {
public:
    EXCEPTION_CLASS(ConfigExceptionIO)

    ConfigExceptionIO(const ConfigOriginPtr& origin,
                      const std::string& message);
    ConfigExceptionIO(const std::string& message);
};

///
/// Exception indicating that there was a parse error.
///
class ConfigExceptionParse : public ConfigException {
public:
    EXCEPTION_CLASS(ConfigExceptionParse)

    ConfigExceptionParse(const ConfigOriginPtr& origin,
                         const std::string& message);
};

///
/// Exception indicating that a substitution did not resolve to anything.
/// Thrown by {@link Config#resolve}.
///
class ConfigExceptionUnresolvedSubstitution : public ConfigExceptionParse {
public:
    EXCEPTION_CLASS(ConfigExceptionUnresolvedSubstitution)

    ConfigExceptionUnresolvedSubstitution(const ConfigOriginPtr& origin,
                                          const std::string& detail);
};

///
/// Exception indicating that you tried to use a function that requires
/// substitutions to be resolved, but substitutions have not been resolved
/// (that is, {@link Config#resolve} was not called). This is always a bug in
/// either application code or the library; it's wrong to write a handler for
/// this exception because you should be able to fix the code to avoid it by
/// adding calls to {@link Config#resolve}.
///
class ConfigExceptionNotResolved : public ConfigExceptionBugOrBroken {
public:
    EXCEPTION_CLASS(ConfigExceptionNotResolved)

    ConfigExceptionNotResolved(const std::string& message);
};

///
/// Information about a problem that occurred in {@link Config#checkValid}. A
/// {@link ConfigExceptionValidationFailed} exception thrown from
/// <code>checkValid()</code> includes a list of problems encountered.
///
class ValidationProblem {
public:
    ValidationProblem(const std::string& path,
                      const ConfigOriginPtr& origin,
                      const std::string& problem);

    /// Returns the config setting causing the problem.
    std::string path();

    /// Returns the config setting causing the problem.
    ConfigOriginPtr origin();

    /// Returns a description of the problem.
    std::string problem();

private:
    std::string path_;
    ConfigOriginPtr origin_;
    std::string problem_;
};

///
/// Exception indicating that {@link Config#checkValid} found validity
/// problems. The problems are available via the {@link #problems()} method.
/// The <code>what()</code> of this exception is a potentially very
/// long string listing all the problems found.
///
class ConfigExceptionValidationFailed : public ConfigException {
public:
    EXCEPTION_CLASS(ConfigExceptionValidationFailed)

    ConfigExceptionValidationFailed(const VectorValidationProblem& problems);

    const VectorValidationProblem& problems();

private:
    static std::string makeMessage(const VectorValidationProblem& problems);

private:
    VectorValidationProblem problems_;
};

///
/// Exception indicating that an action is unsupported.
///
class ConfigExceptionUnsupportedOperation : public ConfigException {
public:
    EXCEPTION_CLASS(ConfigExceptionUnsupportedOperation)

    ConfigExceptionUnsupportedOperation(const std::string& message);
};

///
/// Exception indicating that a file does not exist when attempting to open.
///
class ConfigExceptionFileNotFound : public ConfigExceptionIO {
public:
    EXCEPTION_CLASS(ConfigExceptionFileNotFound)

    ConfigExceptionFileNotFound(const std::string& file);
};

///
/// Exception indicating that a problem occurred during tokenizing.
///
class ConfigExceptionTokenizerProblem : public ConfigException {
public:
    EXCEPTION_CLASS(ConfigExceptionTokenizerProblem)

    ConfigExceptionTokenizerProblem(const TokenPtr& problem);

    TokenPtr problem();

private:
    TokenPtr problem_;
};

///
/// Exception that doesn't fall into any other category.
///
class ConfigExceptionGeneric : public ConfigException {
public:
    EXCEPTION_CLASS(ConfigExceptionGeneric)

    ConfigExceptionGeneric(const std::string& message);
};

}

#endif // CONFIG_EXCEPTION_H_
