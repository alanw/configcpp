/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_H_
#define CONFIG_H_

#include "configcpp/config_mergeable.h"

namespace config {

///
/// An immutable map from config paths to config values.
///
/// <p>
/// Contrast with {@link ConfigObject} which is a map from config <em>keys</em>,
/// rather than paths, to config values. A {@code Config} contains a tree of
/// {@code ConfigObject}, and {@link Config#root()} returns the tree's root
/// object.
///
/// <p>
/// Throughout the API, there is a distinction between "keys" and "paths". A key
/// is a key in a JSON object; it's just a string that's the key in a map. A
/// "path" is a parseable expression with a syntax and it refers to a series of
/// keys. Path expressions are described in the <a
/// href="https://github.com/typesafehub/config/blob/master/HOCON.md">spec for
/// Human-Optimized Config Object Notation</a>. In brief, a path is
/// period-separated so "a.b.c" looks for key c in object b in object a in the
/// root object. Sometimes double quotes are needed around special characters in
/// path expressions.
///
/// <p>
/// The API for a {@code Config} is in terms of path expressions, while the API
/// for a {@code ConfigObject} is in terms of keys. Conceptually, {@code Config}
/// is a one-level map from <em>paths</em> to values, while a
/// {@code ConfigObject} is a tree of nested maps from <em>keys</em> to values.
///
/// <p>
/// Use {@link ConfigUtil#joinPath} and {@link ConfigUtil#splitPath} to convert
/// between path expressions and individual path elements (keys).
///
/// <p>
/// Another difference between {@code Config} and {@code ConfigObject} is that
/// conceptually, {@code ConfigValue}s with a {@link ConfigValue#valueType()
/// valueType()} of {@link ConfigValueType#NONE NONE} exist in a
/// {@code ConfigObject}, while a {@code Config} treats null values as if they
/// were missing.
///
/// <p>
/// {@code Config} is an immutable object and thus safe to use from multiple
/// threads. There's never a need for "defensive copies."
///
/// <p>
/// The "getters" on a {@code Config} all work in the same way. They never return
/// null, nor do they return a {@code ConfigValue} with
/// {@link ConfigValue#valueType() valueType()} of {@link ConfigValueType#NONE
/// NONE}. Instead, they throw {@link ConfigExceptionMissing} if the value is
/// completely absent or set to null. If the value is set to null, a subtype of
/// {@code ConfigExceptionMissing} called {@link ConfigExceptionNull} will be
/// thrown. {@link ConfigExceptionWrongType} will be thrown anytime you ask for
/// a type and the value has an incompatible type. Reasonable type conversions
/// are performed for you though.
///
/// <p>
/// If you want to iterate over the contents of a {@code Config}, you can get its
/// {@code ConfigObject} with {@link #root()}, and then iterate over the
/// {@code ConfigObject} (which implements <code>std::unordered_map</code>). Or,
/// you can use {@link #entrySet()} which recurses the object tree for you and
/// builds up a <code>std::unordered_set</code> of all path-value pairs where the
/// value is not null.
///
/// <p>
/// <em>Do not implement {@code Config}</em>; it should only be implemented by
/// the config library. Arbitrary implementations will not work because the
/// library internals assume a specific concrete implementation. Also, this
/// interface is likely to grow new methods over time, so third-party
/// implementations will break.
///
class Config : public virtual ConfigMergeable {
public:
    /// Loads an application's configuration from the given file basename,
    /// sandwiches it between default reference
    /// config and default overrides, and then resolves it.
    ///
    /// <p>
    /// The loaded object will already be resolved (substitutions have already
    /// been processed). As a result, if you add more fallbacks then they won't
    /// be seen by substitutions. Substitutions are the "${foo.bar}" syntax. If
    /// you want to parse additional files or something then you need to use
    /// {@link #load(Config)}.
    ///
    /// @param fileBasename
    ///            name (optionally without extension) of a config file
    /// @return resolved configuration with overrides and fallbacks added
    static ConfigPtr load(const std::string& fileBasename);

    /// Like {@link #load(const std::string&)} but allows you to specify parse
    /// and resolve options.
    ///
    /// @param fileBasename
    ///            name (optionally without extension) of a config file
    /// @param parseOptions
    ///            options to use when parsing the file
    /// @param resolveOptions
    ///            options to use when resolving the stack
    /// @return resolved configuration with overrides and fallbacks added
    static ConfigPtr load(const std::string& fileBasename,
                          const ConfigParseOptionsPtr& parseOptions,
                          const ConfigResolveOptionsPtr& resolveOptions);

    /// Assembles a standard configuration using a custom <code>Config</code>
    /// object rather than loading "application.conf". The <code>Config</code>
    /// object will be sandwiched between the default reference config and
    /// default overrides and then resolved.
    ///
    /// @param config
    ///            the application's portion of the configuration
    /// @return resolved configuration with overrides and fallbacks added
    static ConfigPtr load(const ConfigPtr& config,
                          const ConfigResolveOptionsPtr& resolveOptions);

    /// Obtains the default override configuration, which currently consists of
    /// environment variables. The returned override configuration will already
    /// have substitutions resolved.
    ///
    /// <p>
    /// The {@link #load()} methods merge this configuration for you
    /// automatically.
    ///
    /// <p>
    /// Future versions may get overrides in more places. It is not guaranteed
    /// that this method <em>only</em> uses system properties.
    ///
    /// @return the default override configuration
    static ConfigPtr defaultOverrides();

    /// Gets an empty configuration with a description to be used to create a
    /// {@link ConfigOrigin} for this <code>Config</code>. The description should
    /// be very short and say what the configuration is, like "default settings"
    /// or "foo settings" or something. (Presumably you will merge some actual
    /// settings into this empty config using {@link Config#withFallback}, making
    /// the description more useful.)
    ///
    /// @param originDescription
    ///            description of the config
    /// @return an empty configuration
    static ConfigPtr emptyConfig(const std::string& originDescription = "");

    static ConfigPtr parseReader(const ReaderPtr& reader,
                                 const ConfigParseOptionsPtr& options = nullptr);

    static ConfigPtr parseFile(const std::string& file,
                               const ConfigParseOptionsPtr& options = nullptr);

    /// Parses a file with a flexible extension. If the <code>fileBasename</code>
    /// already ends in a known extension, this method parses it according to
    /// that extension (the file's syntax must match its extension). If the
    /// <code>fileBasename</code> does not end in an extension, it parses files
    /// with all known extensions and merges whatever is found.
    ///
    /// <p>
    /// In the current implementation, the extension ".conf" forces
    /// {@link ConfigSyntax#CONF}, ".json" forces {@link ConfigSyntax#JSON}, and
    /// ".properties" forces {@link ConfigSyntax#PROPERTIES}. When merging files,
    /// ".conf" falls back to ".json" falls back to ".properties".
    ///
    /// <p>
    /// Future versions of the implementation may add additional syntaxes or
    /// additional extensions. However, the ordering (fallback priority) of the
    /// three current extensions will remain the same.
    ///
    /// <p>
    /// If <code>options</code> forces a specific syntax, this method only parses
    /// files with an extension matching that syntax.
    ///
    /// <p>
    /// If {@link ConfigParseOptions#getAllowMissing options.getAllowMissing()}
    /// is true, then no files have to exist; if false, then at least one file
    /// has to exist.
    ///
    /// @param fileBasename
    ///            a filename with or without extension
    /// @param options
    ///            parse options
    /// @return the parsed configuration
    static ConfigPtr parseFileAnySyntax(const std::string& fileBasename,
                                        const ConfigParseOptionsPtr& options = nullptr);

    static ConfigPtr parseString(const std::string& s,
                                 const ConfigParseOptionsPtr& options = nullptr);

    static ConfigPtr parseMap(const MapVariant& values,
                              const std::string& originDescription = "");

    /// Gets the {@code Config} as a tree of {@link ConfigObject}. This is a
    /// constant-time operation (it is not proportional to the number of values
    /// in the {@code Config}).
    ///
    /// @return the root object in the configuration
    virtual ConfigObjectPtr root() = 0;

    /// Gets the origin of the {@code Config}, which may be a file, or a file
    /// with a line number, or just a descriptive phrase.
    ///
    /// @return the origin of the {@code Config} for use in error messages
    virtual ConfigOriginPtr origin() = 0;

    virtual ConfigMergeablePtr withFallback(const ConfigMergeablePtr& other) = 0;

    /// Returns a replacement config with all substitutions (the
    /// <code>${foo.bar}</code> syntax, see <a
    /// href="https://github.com/typesafehub/config/blob/master/HOCON.md">the
    /// spec</a>) resolved. Substitutions are looked up using this
    /// <code>Config</code> as the root object, that is, a substitution
    /// <code>${foo.bar}</code> will be replaced with the result of
    /// <code>getValue("foo.bar")</code>.
    ///
    /// <p>
    /// This method uses {@link ConfigResolveOptions#defaults()}, there is
    /// another variant {@link Config#resolve(ConfigResolveOptions)} which lets
    /// you specify non-default options.
    ///
    /// <p>
    /// A given {@link Config} must be resolved before using it to retrieve
    /// config values, but ideally should be resolved one time for your entire
    /// stack of fallbacks (see {@link Config#withFallback}). Otherwise, some
    /// substitutions that could have resolved with all fallbacks available may
    /// not resolve, which will be a user-visible oddity.
    ///
    /// <p>
    /// <code>resolve()</code> should be invoked on root config objects, rather
    /// than on a subtree (a subtree is the result of something like
    /// <code>config->getConfig("foo")</code>). The problem with
    /// <code>resolve()</code> on a subtree is that substitutions are relative to
    /// the root of the config and the subtree will have no way to get values
    /// from the root. For example, if you did
    /// <code>config->getConfig("foo")->resolve()</code> on the below config file,
    /// it would not work:
    ///
    /// <pre>
    ///   common-value = 10
    ///   foo {
    ///      whatever = ${common-value}
    ///   }
    /// </pre>
    ///
    /// @return an immutable object with substitutions resolved
    /// @throws ConfigExceptionUnresolvedSubstitution
    ///             if any substitutions refer to nonexistent paths
    /// @throws ConfigException
    ///             some other config exception if there are other problems
    virtual ConfigPtr resolve() = 0;

    /// Like {@link Config#resolve()} but allows you to specify non-default
    /// options.
    ///
    /// @param options
    ///            resolve options
    /// @return the resolved <code>Config</code>
    virtual ConfigPtr resolve(const ConfigResolveOptionsPtr& options) = 0;

    /// Validates this config against a reference config, throwing an exception
    /// if it is invalid. The purpose of this method is to "fail early" with a
    /// comprehensive list of problems; in general, anything this method can find
    /// would be detected later when trying to use the config, but it's often
    /// more user-friendly to fail right away when loading the config.
    ///
    /// <p>
    /// Using this method is always optional, since you can "fail late" instead.
    ///
    /// <p>
    /// You must restrict validation to paths you "own" (those whose meaning are
    /// defined by your code module). If you validate globally, you may trigger
    /// errors about paths that happen to be in the config but have nothing to do
    /// with your module. It's best to allow the modules owning those paths to
    /// validate them. Also, if every module validates only its own stuff, there
    /// isn't as much redundant work being done.
    ///
    /// <p>
    /// If no paths are specified in <code>checkValid()</code>'s parameter list,
    /// validation is for the entire config.
    ///
    /// <p>
    /// If you specify paths that are not in the reference config, those paths
    /// are ignored. (There's nothing to validate.)
    ///
    /// <p>
    /// Here's what validation involves:
    ///
    /// <ul>
    /// <li>All paths found in the reference config must be present in this
    /// config or an exception will be thrown.
    /// <li>
    /// Some changes in type from the reference config to this config will cause
    /// an exception to be thrown. Not all potential type problems are detected,
    /// in particular it's assumed that strings are compatible with everything
    /// except objects and lists. This is because string types are often "really"
    /// some other type (system properties always start out as strings, or a
    /// string like "5ms" could be used with {@link #getMilliseconds}). Also,
    /// it's allowed to set any type to null or override null with any type.
    /// <li>
    /// Any unresolved substitutions in this config will cause a validation
    /// failure; both the reference config and this config should be resolved
    /// before validation. If the reference config is unresolved, it's a bug in
    /// the caller of this method.
    /// </ul>
    ///
    /// <p>
    /// If you want to allow a certain setting to have a flexible type (or
    /// otherwise want validation to be looser for some settings), you could
    /// either remove the problematic setting from the reference config provided
    /// to this method, or you could intercept the validation exception and
    /// screen out certain problems. Of course, this will only work if all other
    /// callers of this method are careful to restrict validation to their own
    /// paths, as they should be.
    ///
    /// <p>
    /// If validation fails, the thrown exception contains a list of all problems
    /// found. See {@link ConfigExceptionValidationFailed#problems}. The
    /// exception's <code>what()</code> will have all the problems
    /// concatenated into one huge string, as well.
    ///
    /// <p>
    /// Again, <code>checkValid()</code> can't guess every domain-specific way a
    /// setting can be invalid, so some problems may arise later when attempting
    /// to use the config. <code>checkValid()</code> is limited to reporting
    /// generic, but common, problems such as missing settings and blatant type
    /// incompatibilities.
    ///
    /// @param reference
    ///            a reference configuration
    /// @param restrictToPaths
    ///            only validate values underneath these paths that your code
    ///            module owns and understands
    /// @throws ConfigExceptionValidationFailed
    ///             if there are any validation issues
    /// @throws ConfigExceptionNotResolved
    ///             if this config is not resolved
    /// @throws ConfigExceptionBugOrBroken
    ///             if the reference config is unresolved or caller otherwise
    ///             misuses the API
    virtual void checkValid(const ConfigPtr& reference,
                            const VectorString& restrictToPaths = VectorString()) = 0;

    /// Checks whether a value is present and non-null at the given path. This
    /// differs in two ways from {@code Map.containsKey()} as implemented by
    /// {@link ConfigObject}: it looks for a path expression, not a key; and it
    /// returns false for null values, while {@code containsKey()} returns true
    /// indicating that the object contains a null value for the key.
    ///
    /// <p>
    /// If a path exists according to {@link #hasPath(std::string)}, then
    /// {@link #getValue(std::string)} will never throw an exception. However, the
    /// typed getters, such as {@link #getInt(std::string)}, will still throw if the
    /// value is not convertible to the requested type.
    ///
    /// @param path
    ///            the path expression
    /// @return true if a non-null value is present at the path
    /// @throws ConfigExceptionBadPath
    ///             if the path expression is invalid
    virtual bool hasPath(const std::string& path) = 0;

    /// Returns true if the {@code Config}'s root object contains no key-value
    /// pairs.
    ///
    /// @return true if the configuration is empty
    virtual bool empty() = 0;

    /// Returns the set of path-value pairs, excluding any null values, found by
    /// recursing {@link #root() the root object}. Note that this is very
    /// different from <code>root()->entrySet()</code> which returns the set of
    /// immediate-child keys in the root object and includes null values.
    ///
    /// @return set of paths with non-null values, built up by recursing the
    ///         entire tree of {@link ConfigObject}
    virtual SetConfigValue entrySet() = 0;

    /// @param path
    ///            path expression
    /// @return the boolean value at the requested path
    /// @throws ConfigExceptionMissing
    ///             if value is absent or null
    /// @throws ConfigExceptionWrongType
    ///             if value is not convertible to boolean
    virtual bool getBoolean(const std::string& path) = 0;

    /// @param path
    ///            path expression
    /// @return the 32-bit integer value at the requested path
    /// @throws ConfigExceptionMissing
    ///             if value is absent or null
    /// @throws ConfigExceptionWrongType
    ///             if value is not convertible to an int (for example it is out
    ///             of range, or it's a boolean value)
    virtual int32_t getInt(const std::string& path) = 0;

    /// @param path
    ///            path expression
    /// @return the 64-bit integer value at the requested path
    /// @throws ConfigExceptionMissing
    ///             if value is absent or null
    /// @throws ConfigExceptionWrongType
    ///             if value is not convertible to an int64
    virtual int64_t getInt64(const std::string& path) = 0;

    /// @param path
    ///            path expression
    /// @return the floating-point value at the requested path
    /// @throws ConfigExceptionMissing
    ///             if value is absent or null
    /// @throws ConfigExceptionWrongType
    ///             if value is not convertible to an double
    virtual double getDouble(const std::string& path) = 0;

    /// @param path
    ///            path expression
    /// @return the string value at the requested path
    /// @throws ConfigExceptionMissing
    ///             if value is absent or null
    /// @throws ConfigExceptionWrongType
    ///             if value is not convertible to an string
    virtual std::string getString(const std::string& path) = 0;

    /// @param path
    ///            path expression
    /// @return the {@link ConfigObject} value at the requested path
    /// @throws ConfigExceptionMissing
    ///             if value is absent or null
    /// @throws ConfigExceptionWrongType
    ///             if value is not convertible to an object
    virtual ConfigObjectPtr getObject(const std::string& path) = 0;

    /// @param path
    ///            path expression
    /// @return the nested {@code Config} value at the requested path
    /// @throws ConfigExceptionMissing
    ///             if value is absent or null
    /// @throws ConfigExceptionWrongType
    ///             if value is not convertible to a Config
    virtual ConfigPtr getConfig(const std::string& path) = 0;

    /// Gets the value at the path as an unwrapped boost::variant value (see
    /// {@link ConfigValue#unwrapped()}).
    ///
    /// @param path
    ///            path expression
    /// @return the unwrapped value at the requested path
    /// @throws ConfigExceptionMissing
    ///             if value is absent or null
    virtual ConfigVariant getVariant(const std::string& path) = 0;

    /// Gets the value at the given path, unless the value is a null value
    /// or missing, in which case it throws just like the other getters. Use
    /// {@code get()} on the {@link Config#root()} object (or other object in
    /// the tree) if you want an unprocessed value.
    ///
    /// @param path
    ///            path expression
    /// @return the value at the requested path
    /// @throws ConfigExceptionMissing
    ///             if value is absent or null
    virtual ConfigValuePtr getValue(const std::string& path) = 0;

    /// Gets a value as a size in bytes (parses special strings like "128M"). If
    /// the value is already a number, then it's left alone; if it's a string,
    /// it's parsed understanding unit suffixes such as "128K", as documented in
    /// the <a
    /// href="https://github.com/typesafehub/config/blob/master/HOCON.md">the
    /// spec</a>.
    ///
    /// @param path
    ///            path expression
    /// @return the value at the requested path, in bytes
    /// @throws ConfigExceptionMissing
    ///             if value is absent or null
    /// @throws ConfigExceptionWrongType
    ///             if value is not convertible to uint64_t or std::string
    /// @throws ConfigExceptionBadValue
    ///             if value cannot be parsed as a size in bytes
    virtual uint64_t getBytes(const std::string& path) = 0;

    /// Get value as a duration in milliseconds. If the value is already a
    /// number, then it's left alone; if it's a string, it's parsed understanding
    /// units suffixes like "10m" or "5ns" as documented in the <a
    /// href="https://github.com/typesafehub/config/blob/master/HOCON.md">the
    /// spec</a>.
    ///
    /// @param path
    ///            path expression
    /// @return the duration value at the requested path, in milliseconds
    /// @throws ConfigExceptionMissing
    ///             if value is absent or null
    /// @throws ConfigExceptionWrongType
    ///             if value is not convertible to uint64_t or std::string
    /// @throws ConfigExceptionBadValue
    ///             if value cannot be parsed as a number of milliseconds
    virtual uint64_t getMilliseconds(const std::string& path) = 0;

    /// Get value as a duration in nanoseconds. If the value is already a number
    /// it's taken as milliseconds and converted to nanoseconds. If it's a
    /// string, it's parsed understanding unit suffixes, as for
    /// {@link #getMilliseconds(std::string)}.
    ///
    /// @param path
    ///            path expression
    /// @return the duration value at the requested path, in nanoseconds
    /// @throws ConfigExceptionMissing
    ///             if value is absent or null
    /// @throws ConfigExceptionWrongType
    ///             if value is not convertible to uint64_t or std::string
    /// @throws ConfigExceptionBadValue
    ///             if value cannot be parsed as a number of nanoseconds
    virtual uint64_t getNanoseconds(const std::string& path) = 0;

    /// Gets a list value (with any element type) as a {@link ConfigList}, which
    /// implements {@code std::vector<ConfigValuePtr>}. Throws if the path is
    /// unset or null.
    ///
    /// @param path
    ///            the path to the list value.
    /// @return the {@link ConfigList} at the path
    /// @throws ConfigExceptionMissing
    ///             if value is absent or null
    /// @throws ConfigExceptionWrongType
    ///             if value is not convertible to a ConfigList
    virtual ConfigListPtr getList(const std::string& path) = 0;

    virtual VectorBool getBooleanList(const std::string& path) = 0;
    virtual VectorInt getIntList(const std::string& path) = 0;
    virtual VectorInt64 getInt64List(const std::string& path) = 0;
    virtual VectorDouble getDoubleList(const std::string& path) = 0;
    virtual VectorString getStringList(const std::string& path) = 0;
    virtual VectorConfigObject getObjectList(const std::string& path) = 0;
    virtual VectorConfig getConfigList(const std::string& path) = 0;
    virtual VectorVariant getVariantList(const std::string& path) = 0;
    virtual VectorInt64 getBytesList(const std::string& path) = 0;
    virtual VectorInt64 getMillisecondsList(const std::string& path) = 0;
    virtual VectorInt64 getNanosecondsList(const std::string& path) = 0;

    /// Clone the config with only the given path (and its children) retained;
    /// all sibling paths are removed.
    ///
    /// @param path
    ///            path to keep
    /// @return a copy of the config minus all paths except the one specified
    virtual ConfigPtr withOnlyPath(const std::string& path) = 0;

    /// Clone the config with the given path removed.
    ///
    /// @param path
    ///            path to remove
    /// @return a copy of the config minus the specified path
    virtual ConfigPtr withoutPath(const std::string& path) = 0;

    /// Places the config inside another {@code Config} at the given path.
    ///
    /// @param path
    ///            path to store this config at.
    /// @return a {@code Config} instance containing this config at the given
    ///         path.
    virtual ConfigPtr atPath(const std::string& path) = 0;

    /// Places the config inside a {@code Config} at the given key. See also
    /// atPath().
    ///
    /// @param key
    ///            key to store this config at.
    /// @return a {@code Config} instance containing this config at the given
    ///         key.
    virtual ConfigPtr atKey(const std::string& key) = 0;
};

}

#endif // CONFIG_H_
