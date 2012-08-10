/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef PARSEABLE_H_
#define PARSEABLE_H_

#include "configcpp/detail/config_base.h"
#include "configcpp/config_parseable.h"

namespace config {

///
/// This is public but it's only for use by the config package; DO NOT TOUCH. The
/// point of this class is to avoid "propagating" each overload on
/// "thing which can be parsed" through multiple interfaces. Most interfaces can
/// have just one overload that takes a Parseable. Also it's used as an abstract
/// "resource handle" in the ConfigIncluder interface.
///
class Parseable : public virtual ConfigParseable, public ConfigBase {
public:
    CONFIG_CLASS(Parseable);

protected:
    Parseable(const ConfigParseOptionsPtr& baseOptions);

public:
    virtual void initialize() override;

private:
    ConfigParseOptionsPtr fixupOptions(const ConfigParseOptionsPtr& baseOptions);

protected:
    void postConstruct(const ConfigParseOptionsPtr& baseOptions);

    /// The general idea is that any work should be in here, not in the
    /// constructor, so that exceptions are thrown from the public parse()
    /// function and not from the creation of the Parseable.
    /// Essentially this is a lazy field. The parser should close the
    /// reader when it's done with it.
    /// ALSO, IMPORTANT: if the file is not found, this must throw
    /// to support the "allow missing" feature.
    virtual ReaderPtr reader() = 0;

    static void trace(const std::string& message);

public:
    virtual ConfigSyntax guessSyntax();

    virtual ConfigParseablePtr relativeTo(const std::string& filename);

    virtual ConfigIncludeContextPtr includeContext();

    static AbstractConfigObjectPtr forceParsedToObject(const ConfigValuePtr& value);

    virtual ConfigObjectPtr parse(const ConfigParseOptionsPtr& options) override;

    virtual AbstractConfigValuePtr parseValue(const ConfigParseOptionsPtr& baseOptions);

private:
    virtual AbstractConfigValuePtr parseValue(const ConfigOriginPtr& origin,
                                              const ConfigParseOptionsPtr& finalOptions);

protected:
    virtual AbstractConfigValuePtr rawParseValue(const ConfigOriginPtr& origin,
                                                 const ConfigParseOptionsPtr& finalOptions);
    virtual AbstractConfigValuePtr rawParseValue(const ReaderPtr& reader,
                                                 const ConfigOriginPtr& origin,
                                                 const ConfigParseOptionsPtr& finalOptions);

public:
    virtual ConfigObjectPtr parse();
    virtual AbstractConfigValuePtr parseValue();

    virtual ConfigOriginPtr origin() override;

protected:
    virtual ConfigOriginPtr createOrigin() = 0;

public:
    virtual ConfigParseOptionsPtr options() override;

    virtual std::string toString() override;

protected:
    static ConfigSyntax syntaxFromExtension(const std::string& name);

public:
    static std::string relativeTo(const std::string& file,
                                  const std::string& filename);

    static ParseablePtr newNotFound(const std::string& whatNotFound,
                                    const std::string& message,
                                    const ConfigParseOptionsPtr& options);
    static ParseablePtr newReader(const ReaderPtr& reader,
                                  const ConfigParseOptionsPtr& options);
    static ParseablePtr newString(const std::string& input,
                                  const ConfigParseOptionsPtr& options);
    static ParseablePtr newFile(const std::string& input,
                                const ConfigParseOptionsPtr& options);

private:
    ConfigParseOptionsPtr baseOptions;
    ConfigIncludeContextPtr includeContext_;
    ConfigParseOptionsPtr initialOptions;
    ConfigOriginPtr initialOrigin;

    /// Note: this tracks the current parse stack and is NOT threadsafe.
    static StackParseable parseStack;

    static const uint32_t MAX_INCLUDE_DEPTH = 50;
};

///
/// This is a parseable that doesn't exist and just throws when you try to
/// parse it.
///
class ParseableNotFound : public Parseable {
public:
    CONFIG_CLASS(ParseableNotFound);

    ParseableNotFound(const std::string& what,
                      const std::string& message,
                      const ConfigParseOptionsPtr& options);

protected:
    virtual ReaderPtr reader() override;
    virtual ConfigOriginPtr createOrigin() override;

private:
    std::string what;
    std::string message;
};

class ParseableReader : public Parseable {
public:
    CONFIG_CLASS(ParseableReader);

    ParseableReader(const ReaderPtr& reader,
                    const ConfigParseOptionsPtr& options);

protected:
    virtual ReaderPtr reader() override;
    virtual ConfigOriginPtr createOrigin() override;

private:
    ReaderPtr reader_;
};

class ParseableString : public Parseable {
public:
    CONFIG_CLASS(ParseableString);

    ParseableString(const std::string& input,
                    const ConfigParseOptionsPtr& options);

protected:
    virtual ReaderPtr reader() override;
    virtual ConfigOriginPtr createOrigin() override;

public:
    virtual std::string toString() override;

private:
    std::string input;
};

class ParseableFile : public Parseable {
public:
    CONFIG_CLASS(ParseableFile);

    ParseableFile(const std::string& input,
                  const ConfigParseOptionsPtr& options);

protected:
    virtual ReaderPtr reader() override;

public:
    virtual ConfigSyntax guessSyntax() override;

    using Parseable::relativeTo;
    virtual ConfigParseablePtr relativeTo(const std::string& filename) override;

protected:
    virtual ConfigOriginPtr createOrigin() override;

public:
    virtual std::string toString() override;

private:
    std::string input;
};

}

#endif // PARSEABLE_H_
