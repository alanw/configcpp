/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef SIMPLE_CONFIG_H_
#define SIMPLE_CONFIG_H_

#include "configcpp/detail/config_base.h"
#include "configcpp/detail/mergeable_value.h"
#include "configcpp/config.h"

namespace config {

///
/// One thing to keep in mind in the future: as Collection-like APIs are added
/// here, including iterators or size() or anything, they should be consistent
/// with a one-level std::unordered_map from paths to non-null values. Null
/// values are not "in" the map.
///
class SimpleConfig : public virtual Config, public virtual MergeableValue, public ConfigBase {
public:
    CONFIG_CLASS(SimpleConfig);

    SimpleConfig(const AbstractConfigObjectPtr& object);

    virtual ConfigObjectPtr root() override;
    virtual ConfigOriginPtr origin() override;
    virtual ConfigPtr resolve() override;
    virtual ConfigPtr resolve(const ConfigResolveOptionsPtr& options) override;
    virtual bool hasPath(const std::string& path) override;
    virtual bool empty() override;

private:
    static void findPaths(SetConfigValue& entries,
                          const PathPtr& parent,
                          const AbstractConfigObjectPtr& obj);

public:
    virtual SetConfigValue entrySet() override;

private:
    static AbstractConfigValuePtr findKey(const AbstractConfigObjectPtr& self,
                                          const std::string& key,
                                          ConfigValueType expected,
                                          const PathPtr& originalPath);
    static AbstractConfigValuePtr find(const AbstractConfigObjectPtr& self,
                                       const PathPtr& path,
                                       ConfigValueType expected,
                                       const PathPtr& originalPath);

    AbstractConfigValuePtr find(const PathPtr& pathExpression,
                                ConfigValueType expected,
                                const PathPtr& originalPath);
    AbstractConfigValuePtr find(const std::string& path,
                                ConfigValueType expected);

public:
    virtual ConfigValuePtr getValue(const std::string& path) override;
    virtual bool getBoolean(const std::string& path) override;
    virtual ConfigNumberPtr getConfigNumber(const std::string& path);
    virtual int32_t getInt(const std::string& path) override;
    virtual int64_t getInt64(const std::string& path) override;
    virtual double getDouble(const std::string& path) override;
    virtual std::string getString(const std::string& path) override;
    virtual ConfigListPtr getList(const std::string& path) override;
    virtual ConfigObjectPtr getObject(const std::string& path) override;
    virtual ConfigPtr getConfig(const std::string& path) override;
    virtual ConfigVariant getVariant(const std::string& path) override;
    virtual uint64_t getBytes(const std::string& path) override;
    virtual uint64_t getMilliseconds(const std::string& path) override;
    virtual uint64_t getNanoseconds(const std::string& path) override;

private:
    VectorVariant getHomogeneousUnwrappedList(const std::string& path,
                                              ConfigValueType expected);

public:
    virtual VectorBool getBooleanList(const std::string& path) override;
    virtual VectorInt getIntList(const std::string& path) override;
    virtual VectorInt64 getInt64List(const std::string& path) override;
    virtual VectorDouble getDoubleList(const std::string& path) override;
    virtual VectorString getStringList(const std::string& path) override;

private:
    VectorConfigValue getHomogeneousWrappedList(const std::string& path,
                                                ConfigValueType expected);

public:
    virtual VectorConfigObject getObjectList(const std::string& path) override;
    virtual VectorConfig getConfigList(const std::string& path) override;
    virtual VectorVariant getVariantList(const std::string& path) override;
    virtual VectorInt64 getBytesList(const std::string& path) override;
    virtual VectorInt64 getMillisecondsList(const std::string& path) override;
    virtual VectorInt64 getNanosecondsList(const std::string& path) override;

    virtual ConfigValuePtr toFallbackValue() override;
    virtual ConfigMergeablePtr withFallback(const ConfigMergeablePtr& other) override;

    virtual bool equals(const ConfigVariant& other) override;
    virtual uint32_t hashCode() override;
    virtual std::string toString() override;

private:
    static std::string getUnits(const std::string& s);

public:
    /// Parses a duration string. If no units are specified in the string, it is
    /// assumed to be in milliseconds. The returned duration is in nanoseconds.
    /// The purpose of this function is to implement the duration-related methods
    /// in the ConfigObject interface.
    ///
    /// @param input
    ///            the string to parse
    /// @param originForException
    ///            origin of the value being parsed
    /// @param pathForException
    ///            path to include in exceptions
    /// @return duration in nanoseconds
    /// @throws ConfigException
    ///             if string is invalid
    static uint64_t parseDuration(const std::string& input,
                                  const ConfigOriginPtr& originForException,
                                  const std::string& pathForException);

    /// Parses a size-in-bytes string. If no units are specified in the string,
    /// it is assumed to be in bytes. The returned value is in bytes. The purpose
    /// of this function is to implement the size-in-bytes-related methods in the
    /// Config interface.
    ///
    /// @param input
    ///            the string to parse
    /// @param originForException
    ///            origin of the value being parsed
    /// @param pathForException
    ///            path to include in exceptions
    /// @return size in bytes
    /// @throws ConfigException
    ///             if string is invalid
    static uint64_t parseBytes(const std::string& input,
                               const ConfigOriginPtr& originForException,
                               const std::string& pathForException);

private:
    virtual AbstractConfigValuePtr peekPath(const PathPtr& path);

    static void addProblem(VectorValidationProblem& accumulator,
                           const PathPtr& path,
                           const ConfigOriginPtr& origin,
                           const std::string& problem);

    static std::string getDesc(const ConfigValuePtr& refValue);

    static void addMissing(VectorValidationProblem& accumulator,
                           const ConfigValuePtr& refValue,
                           const PathPtr& path,
                           const ConfigOriginPtr& origin);
    static void addWrongType(VectorValidationProblem& accumulator,
                             const ConfigValuePtr& refValue,
                             const AbstractConfigValuePtr& actual,
                             const PathPtr& path);

    static bool couldBeNull(const AbstractConfigValuePtr& v);

    static bool haveCompatibleTypes(const ConfigValuePtr& reference,
                                    const AbstractConfigValuePtr& value);

    static void checkValidObject(const PathPtr& path,
                                 const AbstractConfigObjectPtr& reference,
                                 const AbstractConfigObjectPtr& value,
                                 VectorValidationProblem& accumulator);
    static void checkValid(const PathPtr& path,
                           const ConfigValuePtr& reference,
                           const AbstractConfigValuePtr& value,
                           VectorValidationProblem& accumulator);

    virtual void checkValid(const ConfigPtr& reference,
                            const VectorString& restrictToPaths = VectorString()) override;

    virtual ConfigPtr withOnlyPath(const std::string& path) override;
    virtual ConfigPtr withoutPath(const std::string& path) override;

private:
    AbstractConfigObjectPtr object;
};

class MemoryUnit {
public:
    MemoryUnit(const std::string& prefix = "", uint32_t powerOf = 0, uint32_t power = 0);

private:
    typedef std::unordered_map<std::string, MemoryUnit> MapMemoryUnit;
    static MapMemoryUnit makeUnitsMap();

public:
    static MemoryUnit parseUnit(const std::string& unit);
    static bool isNull(const MemoryUnit& unit);

public:
    std::string prefix;
    uint32_t powerOf;
    uint32_t power;
    uint64_t bytes;
};

}

#endif // SIMPLE_CONFIG_H_

