/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_TYPES_H_
#define CONFIG_TYPES_H_

#include <sys/stat.h>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <set>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <functional>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include <limits>
#include <cstring>
#include <type_traits>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/blank.hpp>
#include <boost/functional/hash.hpp>
#include <boost/optional.hpp>

// Fix compatibility with gcc 4.7
#define BOOST_VARIANT_NO_FULL_RECURSIVE_VARIANT_SUPPORT
#include <boost/variant.hpp>

// Force boost file-system version 2 for later boost versions > 1.46
#define BOOST_FILESYSTEM_VERSION 2
#include <boost/filesystem/path.hpp>

namespace config {

enum class ConfigSyntax : uint32_t;
enum class ConfigValueType : uint32_t;
enum class FromMapMode : uint32_t;
enum class OriginType : uint32_t;
enum class TokenType : uint32_t;

#define DECLARE_SHARED_PTR(Type) \
    class Type; \
    typedef std::shared_ptr<Type> Type##Ptr;

DECLARE_SHARED_PTR(AbstractConfigValue)
DECLARE_SHARED_PTR(AbstractConfigObject)
DECLARE_SHARED_PTR(Config)
DECLARE_SHARED_PTR(ConfigBase)
DECLARE_SHARED_PTR(ConfigException)
DECLARE_SHARED_PTR(ConfigIncludeContext)
DECLARE_SHARED_PTR(ConfigIncluder)
DECLARE_SHARED_PTR(ConfigMergeable)
DECLARE_SHARED_PTR(ConfigNumber)
DECLARE_SHARED_PTR(ConfigObject)
DECLARE_SHARED_PTR(ConfigList)
DECLARE_SHARED_PTR(ConfigOrigin)
DECLARE_SHARED_PTR(ConfigParseable)
DECLARE_SHARED_PTR(ConfigParseOptions)
DECLARE_SHARED_PTR(ConfigReference)
DECLARE_SHARED_PTR(ConfigRenderOptions)
DECLARE_SHARED_PTR(ConfigResolveOptions)
DECLARE_SHARED_PTR(ConfigValue)
DECLARE_SHARED_PTR(Element)
DECLARE_SHARED_PTR(FullIncluder)
DECLARE_SHARED_PTR(Parseable)
DECLARE_SHARED_PTR(Parser)
DECLARE_SHARED_PTR(Path)
DECLARE_SHARED_PTR(PathBuilder)
DECLARE_SHARED_PTR(Unmergeable)
DECLARE_SHARED_PTR(MemoKey)
DECLARE_SHARED_PTR(Modifier)
DECLARE_SHARED_PTR(NameSource)
DECLARE_SHARED_PTR(NoExceptionsModifier)
DECLARE_SHARED_PTR(Reader)
DECLARE_SHARED_PTR(ReplaceableMergeStack)
DECLARE_SHARED_PTR(ResolveContext)
DECLARE_SHARED_PTR(ResolveMemos)
DECLARE_SHARED_PTR(ResolveReplacer)
DECLARE_SHARED_PTR(ResolveSource)
DECLARE_SHARED_PTR(SimpleConfig)
DECLARE_SHARED_PTR(SimpleConfigList)
DECLARE_SHARED_PTR(SimpleConfigObject)
DECLARE_SHARED_PTR(SimpleConfigOrigin)
DECLARE_SHARED_PTR(SimpleIncludeContext)
DECLARE_SHARED_PTR(SubstitutionExpression)
DECLARE_SHARED_PTR(Token)
DECLARE_SHARED_PTR(TokenIterator)
DECLARE_SHARED_PTR(TokenWithComments)
DECLARE_SHARED_PTR(ValidationProblem)
DECLARE_SHARED_PTR(WhitespaceSaver)

typedef std::pair<std::string, ConfigValuePtr> PairStringValue;

struct null : public boost::blank {};

typedef boost::make_recursive_variant<
    null,
    int32_t,
    int64_t,
    double,
    bool,
    std::string,
    ConfigBasePtr,
    std::unordered_map<std::string, boost::recursive_variant_>,
    std::vector<boost::recursive_variant_>
>::type ConfigVariant;

}

namespace std {

template <> class hash<config::PairStringValue> {
public:
    std::size_t operator()(const config::PairStringValue& v) const {
        size_t hash = 0;
        boost::hash_combine(hash, std::hash<std::string>()(v.first));
        boost::hash_combine(hash, std::hash<void*>()(v.second.get()));
        return hash;
    }
};

template <> class hash<config::ConfigVariant>;

}

namespace config {

template <class T>
struct configHash : std::unary_function<T, std::size_t> {
    inline std::size_t operator()(const T& type) const {
        return type ? type->hashCode() : 0;
    }
};

template <class T>
struct configEquals {
    inline bool operator()(const T& first, const T& second) const {
        return !first ? !second : first->equals(second);
    }
};

typedef std::unordered_set<std::string> SetString;
typedef std::unordered_set<PairStringValue> SetConfigValue;
typedef std::unordered_set<PathPtr, configHash<PathPtr>, configEquals<PathPtr>> SetPath;

typedef std::unordered_map<std::string, ConfigVariant> MapVariant;
typedef std::unordered_map<std::string, std::string> MapString;
typedef std::unordered_map<std::string, ConfigValuePtr> MapConfigValue;
typedef std::unordered_map<std::string, AbstractConfigValuePtr> MapAbstractConfigValue;
typedef std::unordered_map<MemoKeyPtr, AbstractConfigValuePtr, configHash<MemoKeyPtr>, configEquals<MemoKeyPtr>> MapMemoKeyAbstractConfigValue;
typedef std::unordered_map<AbstractConfigValuePtr, ResolveReplacerPtr, configHash<AbstractConfigValuePtr>> MapResolveReplacer;
typedef std::unordered_map<PathPtr, ConfigVariant, configHash<PathPtr>, configEquals<PathPtr>> MapPathVariant;
typedef std::unordered_map<PathPtr, MapAbstractConfigValue, configHash<PathPtr>, configEquals<PathPtr>> MapPathMapAbstractConfigValue;

typedef std::deque<ParseablePtr> StackParseable;
typedef std::deque<std::string> StackString;
typedef std::deque<TokenPtr> QueueToken;
typedef std::deque<TokenWithCommentsPtr> StackTokenWithComments;
typedef std::deque<int32_t> QueueInt;
typedef std::deque<PathPtr> StackPath;

typedef std::vector<std::string> VectorString;
typedef std::vector<bool> VectorBool;
typedef std::vector<int32_t> VectorInt;
typedef std::vector<int64_t> VectorInt64;
typedef std::vector<double> VectorDouble;
typedef std::vector<ConfigObjectPtr> VectorConfigObject;
typedef std::vector<ConfigPtr> VectorConfig;
typedef std::vector<ConfigOriginPtr> VectorConfigOrigin;
typedef std::vector<ConfigValuePtr> VectorConfigValue;
typedef std::vector<ConfigVariant> VectorVariant;
typedef std::vector<AbstractConfigValuePtr> VectorAbstractConfigValue;
typedef std::vector<AbstractConfigObjectPtr> VectorAbstractConfigObject;
typedef std::vector<ValidationProblem> VectorValidationProblem;
typedef std::vector<PathPtr> VectorPath;
typedef std::vector<TokenPtr> VectorToken;
typedef std::vector<ElementPtr> VectorElement;
typedef std::vector<SubstitutionExpressionPtr> VectorSubstitutionExpression;

}

#endif // CONFIG_TYPES_H_
