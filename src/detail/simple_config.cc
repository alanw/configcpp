/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/simple_config.h"
#include "configcpp/detail/simple_config_list.h"
#include "configcpp/detail/abstract_config_object.h"
#include "configcpp/detail/abstract_config_value.h"
#include "configcpp/detail/resolve_context.h"
#include "configcpp/detail/path.h"
#include "configcpp/detail/config_impl.h"
#include "configcpp/detail/config_null.h"
#include "configcpp/detail/config_number.h"
#include "configcpp/detail/config_string.h"
#include "configcpp/detail/default_transformer.h"
#include "configcpp/config_resolve_options.h"
#include "configcpp/config_exception.h"
#include "configcpp/config_value_type.h"
#include "configcpp/config_list.h"

namespace config {

SimpleConfig::SimpleConfig(const AbstractConfigObjectPtr& object) :
    object(object) {
}

ConfigObjectPtr SimpleConfig::root() {
    return object;
}

ConfigOriginPtr SimpleConfig::origin() {
    return object->origin();
}

ConfigPtr SimpleConfig::resolve() {
    return resolve(ConfigResolveOptions::defaults());
}

ConfigPtr SimpleConfig::resolve(const ConfigResolveOptionsPtr& options) {
    auto resolved = ResolveContext::resolve(object, object, options);

    if (resolved == object) {
        return shared_from_this();
    }
    else {
        return make_instance(std::static_pointer_cast<AbstractConfigObject>(resolved));
    }
}

bool SimpleConfig::hasPath(const std::string& pathExpression) {
    auto path = Path::newPath(pathExpression);
    ConfigValuePtr peeked;
    try {
        peeked = object->peekPath(path);
    }
    catch (ConfigExceptionNotResolved& e) {
        throw ConfigImpl::improveNotResolved(path, e);
    }
    return peeked && peeked->valueType() != ConfigValueType::NONE;
}

bool SimpleConfig::empty() {
    return object->empty();
}

void SimpleConfig::findPaths(SetConfigValue& entries, const PathPtr& parent, const AbstractConfigObjectPtr& obj) {
    for (auto& entry : obj->entrySet()) {
        std::string elem = entry.first;
        auto v = entry.second;
        auto path = Path::newKey(elem);
        if (parent) {
            path = path->prepend(parent);
        }
        if (instanceof<AbstractConfigObject>(v)) {
            findPaths(entries, path, std::dynamic_pointer_cast<AbstractConfigObject>(v));
        }
        else if (instanceof<ConfigNull>(v)) {
            // nothing; nulls are conceptually not in a Config
        }
        else {
            entries.insert(std::make_pair(path->render(), v));
        }
    }
}

SetConfigValue SimpleConfig::entrySet() {
    SetConfigValue entries;
    findPaths(entries, nullptr, object);
    return entries;
}

AbstractConfigValuePtr SimpleConfig::findKey(const AbstractConfigObjectPtr& self, const std::string& key, ConfigValueType expected, const PathPtr& originalPath) {
    auto v = self->peekAssumingResolved(key, originalPath);
    if (!v) {
        throw ConfigExceptionMissing(originalPath->render());
    }
    if (expected != ConfigValueType::NONE) {
        v = DefaultTransformer::transform(v, expected);
    }
    if (v->valueType() == ConfigValueType::NONE) {
        throw ConfigExceptionNull(
            v->origin(),
            originalPath->render(),
            expected != ConfigValueType::NONE ? ConfigValueTypeEnum::name(expected) : "");
    }
    else if (expected != ConfigValueType::NONE && v->valueType() != expected) {
        throw ConfigExceptionWrongType(
            v->origin(),
            originalPath->render(),
            ConfigValueTypeEnum::name(expected),
            ConfigValueTypeEnum::name(v->valueType()));
    }
    else {
        return v;
    }
}

AbstractConfigValuePtr SimpleConfig::find(const AbstractConfigObjectPtr& self, const PathPtr& path, ConfigValueType expected, const PathPtr& originalPath) {
    try {
        std::string key = path->first();
        auto next = path->remainder();
        if (!next) {
            return findKey(self, key, expected, originalPath);
        }
        else {
            auto o = std::static_pointer_cast<AbstractConfigObject>(findKey(self, key, ConfigValueType::OBJECT, originalPath->subPath(0, originalPath->length() - next->length())));
            assert(o); // missing was supposed to throw
            return find(o, next, expected, originalPath);
        }
    }
    catch (ConfigExceptionNotResolved& e) {
        throw ConfigImpl::improveNotResolved(path, e);
    }
}

AbstractConfigValuePtr SimpleConfig::find(const PathPtr& pathExpression, ConfigValueType expected, const PathPtr& originalPath) {
    return find(object, pathExpression, expected, originalPath);
}

AbstractConfigValuePtr SimpleConfig::find(const std::string& pathExpression, ConfigValueType expected) {
    auto path = Path::newPath(pathExpression);
    return find(path, expected, path);
}

ConfigValuePtr SimpleConfig::getValue(const std::string& path) {
    return find(path, ConfigValueType::NONE);
}

bool SimpleConfig::getBoolean(const std::string& path) {
    auto v = find(path, ConfigValueType::BOOLEAN);
    return v->unwrapped<bool>();
}

ConfigNumberPtr SimpleConfig::getConfigNumber(const std::string& path) {
    auto v = find(path, ConfigValueType::NUMBER);
    return std::dynamic_pointer_cast<ConfigNumber>(v);
}

int32_t SimpleConfig::getInt(const std::string& path) {
    auto n = getConfigNumber(path);
    return n->intValueRangeChecked(path);
}

int64_t SimpleConfig::getInt64(const std::string& path) {
    auto u = getConfigNumber(path)->unwrapped();
    return boost::apply_visitor(VariantInt64(), u);
}

double SimpleConfig::getDouble(const std::string& path) {
    auto u = getConfigNumber(path)->unwrapped();
    return boost::apply_visitor(VariantDouble(), u);
}

std::string SimpleConfig::getString(const std::string& path) {
    auto v = find(path, ConfigValueType::STRING);
    return v->unwrapped<std::string>();
}

ConfigListPtr SimpleConfig::getList(const std::string& path) {
    auto v = find(path, ConfigValueType::LIST);
    return std::dynamic_pointer_cast<ConfigList>(v);
}

ConfigObjectPtr SimpleConfig::getObject(const std::string& path) {
    auto obj = std::static_pointer_cast<AbstractConfigObject>(find(path, ConfigValueType::OBJECT));
    return obj;
}

ConfigPtr SimpleConfig::getConfig(const std::string& path) {
    return getObject(path)->toConfig();
}

ConfigVariant SimpleConfig::getVariant(const std::string& path) {
    auto v = find(path, ConfigValueType::NONE);
    return v->unwrapped();
}

uint64_t SimpleConfig::getBytes(const std::string& path) {
    uint64_t size = 0;
    try {
        size = getInt64(path);
    }
    catch (ConfigExceptionWrongType&) {
        auto v = find(path, ConfigValueType::STRING);
        size = parseBytes(v->unwrapped<std::string>(), v->origin(), path);
    }
    return size;
}

uint64_t SimpleConfig::getMilliseconds(const std::string& path) {
    return getNanoseconds(path) / 1000000;
}

uint64_t SimpleConfig::getNanoseconds(const std::string& path) {
    uint64_t size = 0;
    try {
        size = getInt64(path) * 1000000;
    }
    catch (ConfigExceptionWrongType&) {
        auto v = find(path, ConfigValueType::STRING);
        size = parseDuration(v->unwrapped<std::string>(), v->origin(), path);
    }
    return size;
}

VectorVariant SimpleConfig::getHomogeneousUnwrappedList(const std::string& path, ConfigValueType expected) {
    VectorVariant variantList;
    auto list = getList(path);
    variantList.reserve(list->size());
    for (auto& cv : *list) {
        // variance would be nice, but stupid cast will do
        auto v = std::dynamic_pointer_cast<AbstractConfigValue>(cv);
        if (expected != ConfigValueType::NONE) {
            v = DefaultTransformer::transform(v, expected);
        }
        if (v->valueType() != expected) {
            throw ConfigExceptionWrongType(
                v->origin(),
                path,
                "list of " + ConfigValueTypeEnum::name(expected),
                "list of " + ConfigValueTypeEnum::name(v->valueType()));
        }
        variantList.push_back(v->unwrapped());
    }
    return variantList;
}

VectorBool SimpleConfig::getBooleanList(const std::string& path) {
    VectorVariant list = getHomogeneousUnwrappedList(path, ConfigValueType::BOOLEAN);
    VectorBool boolList;
    boolList.reserve(list.size());
    for (auto& v : list) {
        boolList.push_back(variant_get<bool>(v));
    }
    return boolList;
}

VectorInt SimpleConfig::getIntList(const std::string& path) {
    VectorConfigValue list = getHomogeneousWrappedList(path, ConfigValueType::NUMBER);
    VectorInt intList;
    intList.reserve(list.size());
    for (auto& v : list) {
        intList.push_back(std::dynamic_pointer_cast<ConfigNumber>(v)->intValueRangeChecked(path));
    }
    return intList;
}

VectorInt64 SimpleConfig::getInt64List(const std::string& path) {
    VectorVariant list = getHomogeneousUnwrappedList(path, ConfigValueType::NUMBER);
    VectorInt64 int64List;
    int64List.reserve(list.size());
    for (auto& v : list) {
        int64List.push_back(boost::apply_visitor(VariantInt64(), v));
    }
    return int64List;
}

VectorDouble SimpleConfig::getDoubleList(const std::string& path) {
    VectorVariant list = getHomogeneousUnwrappedList(path, ConfigValueType::NUMBER);
    VectorDouble doubleList;
    doubleList.reserve(list.size());
    for (auto& v : list) {
        doubleList.push_back(boost::apply_visitor(VariantDouble(), v));
    }
    return doubleList;
}

VectorString SimpleConfig::getStringList(const std::string& path) {
    VectorVariant list = getHomogeneousUnwrappedList(path, ConfigValueType::STRING);
    VectorString stringList;
    stringList.reserve(list.size());
    for (auto& v : list) {
        stringList.push_back(variant_get<std::string>(v));
    }
    return stringList;
}

VectorConfigValue SimpleConfig::getHomogeneousWrappedList(const std::string& path, ConfigValueType expected) {
    VectorConfigValue wrappedList;
    auto list = getList(path);
    wrappedList.reserve(list->size());
    for (auto& cv : *list) {
        // variance would be nice, but stupid cast will do
        auto v = std::dynamic_pointer_cast<AbstractConfigValue>(cv);
        if (expected != ConfigValueType::NONE) {
            v = DefaultTransformer::transform(v, expected);
        }
        if (v->valueType() != expected) {
            throw ConfigExceptionWrongType(
                v->origin(),
                path,
                "list of " + ConfigValueTypeEnum::name(expected),
                "list of " + ConfigValueTypeEnum::name(v->valueType()));
        }
        wrappedList.push_back(v);
    }
    return wrappedList;
}

VectorConfigObject SimpleConfig::getObjectList(const std::string& path) {
    VectorConfigValue list = getHomogeneousWrappedList(path, ConfigValueType::OBJECT);
    VectorConfigObject objectList;
    objectList.reserve(list.size());
    for (auto& v : list) {
        objectList.push_back(std::dynamic_pointer_cast<ConfigObject>(v));
    }
    return objectList;
}

VectorConfig SimpleConfig::getConfigList(const std::string& path) {
    VectorVariant list = getHomogeneousUnwrappedList(path, ConfigValueType::OBJECT);
    VectorConfig configList;
    configList.reserve(list.size());
    for (auto& v : list) {
        configList.push_back(dynamic_get<ConfigObject>(v)->toConfig());
    }
    return configList;
}

VectorVariant SimpleConfig::getVariantList(const std::string& path) {
    VectorVariant variantList;
    auto list = getList(path);
    variantList.reserve(list->size());
    for (auto& v : *list) {
        variantList.push_back(v->unwrapped());
    }
    return variantList;
}

VectorInt64 SimpleConfig::getBytesList(const std::string& path) {
    VectorInt64 int64List;
    auto list = getList(path);
    int64List.reserve(list->size());
    for (auto& v : *list) {
        if (v->valueType() == ConfigValueType::NUMBER) {
            auto u = v->unwrapped();
            int64List.push_back(boost::apply_visitor(VariantInt64(), u));
        }
        else if (v->valueType() == ConfigValueType::STRING) {
            std::string s = v->unwrapped<std::string>();
            int64List.push_back(parseBytes(s, v->origin(), path));
        }
        else {
            throw ConfigExceptionWrongType(
                v->origin(),
                path,
                "memory size string or number of bytes",
                ConfigValueTypeEnum::name(v->valueType()));
        }
    }
    return int64List;
}

VectorInt64 SimpleConfig::getMillisecondsList(const std::string& path) {
    VectorInt64 nanos = getNanosecondsList(path);
    VectorInt64 int64List;
    int64List.reserve(nanos.size());
    for (auto& nano : nanos) {
        int64List.push_back(nano / 1000000);
    }
    return int64List;
}

VectorInt64 SimpleConfig::getNanosecondsList(const std::string& path) {
    VectorInt64 int64List;
    auto list = getList(path);
    int64List.reserve(list->size());
    for (auto& v : *list) {
        if (v->valueType() == ConfigValueType::NUMBER) {
            auto u = v->unwrapped();
            int64List.push_back(boost::apply_visitor(VariantInt64(), u) * 1000000LL);
        }
        else if (v->valueType() == ConfigValueType::STRING) {
            std::string s = v->unwrapped<std::string>();
            int64List.push_back(parseDuration(s, v->origin(), path));
        }
        else {
            throw ConfigExceptionWrongType(
                v->origin(),
                path,
                "duration string or number of nanoseconds",
                ConfigValueTypeEnum::name(v->valueType()));
        }
    }
    return int64List;
}

ConfigValuePtr SimpleConfig::toFallbackValue() {
    return object;
}

ConfigMergeablePtr SimpleConfig::withFallback(const ConfigMergeablePtr& other) {
    // this can return "this" if the withFallback doesn't need a new ConfigObject
    return std::dynamic_pointer_cast<AbstractConfigObject>(object->withFallback(other))->toConfig();
}

bool SimpleConfig::equals(const ConfigVariant& other) {
    if (instanceof<SimpleConfig>(other)) {
        return object->equals(static_get<SimpleConfig>(other)->object);
    }
    else {
        return false;
    }
}

uint32_t SimpleConfig::hashCode() {
    // we do the "41*" just so our hash code won't match that of the
    // underlying object. there's no real reason it can't match, but
    // making it not match might catch some kinds of bug.
    return 41 * object->hashCode();
}

std::string SimpleConfig::toString() {
    return "Config(" + object->toString() + ")";
}

std::string SimpleConfig::getUnits(const std::string& s) {
    for (auto i = s.rbegin(); i != s.rend(); ++i) {
        if (!std::isalpha(*i)) {
            return std::string(i.base(), s.end());
        }
    }
    return s;
}

uint64_t SimpleConfig::parseDuration(const std::string& input, const ConfigOriginPtr& originForException, const std::string& pathForException) {
    std::string s = boost::trim_copy(input);
    std::string originalUnitString = getUnits(s);
    std::string unitString = originalUnitString;
    std::string numberString = boost::trim_copy(s.substr(0, s.length() - unitString.length()));
    uint64_t units = 0;

    // this would be caught later anyway, but the error message
    // is more helpful if we check it here.
    if (numberString.empty()) {
        throw ConfigExceptionBadValue(
            originForException,
            pathForException,
            "No number in duration value '" + input + "'");
    }

    if (unitString.length() > 2 && !boost::ends_with(unitString, "s")) {
        unitString += "s";
    }

    // note that this is deliberately case-sensitive
    if (unitString == "" || unitString == "ms" || unitString == "milliseconds") {
        units = 1000000LL;
    }
    else if (unitString == "us" || unitString == "microseconds") {
        units = 1000LL;
    }
    else if (unitString == "ns" || unitString == "nanoseconds") {
        units = 1LL;
    }
    else if (unitString == "d" || unitString == "days") {
        units = 86400000000000LL;
    }
    else if (unitString == "h" || unitString == "hrs" || unitString == "hours") {
        units = 3600000000000LL;
    }
    else if (unitString == "s" || unitString == "secs" || unitString == "seconds") {
        units = 1000000000LL;
    }
    else if (unitString == "m" || unitString == "mins" || unitString == "minutes") {
        units = 60000000000LL;
    }
    else {
        throw ConfigExceptionBadValue(
            originForException,
            pathForException,
            "Could not parse time unit '" + originalUnitString + "' (try ns, us, ms, s, m, d)");
    }

    try {
        // if the string is purely digits, parse as an integer to avoid
        // possible precision loss; otherwise as a double.
        if (std::all_of(numberString.begin(), numberString.end(), (int(*)(int))std::isdigit)) {
            return boost::lexical_cast<int64_t>(numberString) * units;
        }
        else {
            return static_cast<int64_t>(boost::lexical_cast<double>(numberString) * units);
        }
    }
    catch (boost::bad_lexical_cast&) {
        throw ConfigExceptionBadValue(
            originForException,
            pathForException,
            "Could not parse duration number '" + numberString + "'");
    }
}

MemoryUnit::MemoryUnit(const std::string& prefix, uint32_t powerOf, uint32_t power) :
    prefix(prefix),
    powerOf(powerOf),
    power(power),
    bytes(1) {
    for (uint32_t i = power; i > 0; --i) {
        bytes *= powerOf;
    }
}

MemoryUnit::MapMemoryUnit MemoryUnit::makeUnitsMap() {
    typedef std::vector<MemoryUnit> VectorUnit;
    static VectorUnit units = {
        {"", 1024, 0},

        {"kilo", 1000, 1},
        {"mega", 1000, 2},
        {"giga", 1000, 3},
        {"tera", 1000, 4},
        {"peta", 1000, 5},
        {"exa", 1000, 6},
        {"zetta", 1000, 7},
        {"yotta", 1000, 8},

        {"kibi", 1024, 1},
        {"mebi", 1024, 2},
        {"gibi", 1024, 3},
        {"tebi", 1024, 4},
        {"pebi", 1024, 5},
        {"exbi", 1024, 6},
        {"zebi", 1024, 7},
        {"yobi", 1024, 8}
    };
    MapMemoryUnit map;
    for (auto& unit : units) {
        map[unit.prefix + "byte"] = unit;
        map[unit.prefix + "bytes"] = unit;
        if (unit.prefix.empty()) {
            map["b"] = unit;
            map["B"] = unit;
            map[""] = unit; // no unit specified means bytes
        }
        else {
            std::string first = unit.prefix.substr(0, 1);
            std::string firstUpper = boost::to_upper_copy(first);
            if (unit.powerOf == 1024) {
                map[first] = unit; // 512m
                map[firstUpper] = unit; // 512M
                map[firstUpper + "i"] = unit; // 512Mi
                map[firstUpper + "iB"] = unit; // 512MiB
            }
            else if (unit.powerOf == 1000) {
                if (unit.power == 1) {
                    map[first + "B"] = unit; // 512kB
                }
                else {
                    map[firstUpper + "B"] = unit; // 512MB
                }
            }
            else {
                throw std::runtime_error("broken MemoryUnit enum");
            }
        }
    }
    return map;
}

MemoryUnit MemoryUnit::parseUnit(const std::string& unit) {
    static MapMemoryUnit unitsMap = makeUnitsMap();
    auto memoryUnit = unitsMap.find(unit);
    return memoryUnit == unitsMap.end() ? MemoryUnit() : memoryUnit->second;
}

bool MemoryUnit::isNull(const MemoryUnit& unit) {
    return unit.power == 0 && unit.powerOf == 0;
}

uint64_t SimpleConfig::parseBytes(const std::string& input, const ConfigOriginPtr& originForException, const std::string& pathForException) {
    std::string s = boost::trim_copy(input);
    std::string unitString = getUnits(s);
    std::string numberString = boost::trim_copy(s.substr(0, s.length() - unitString.length()));

    // this would be caught later anyway, but the error message
    // is more helpful if we check it here.
    if (numberString.empty()) {
        throw ConfigExceptionBadValue(
            originForException,
            pathForException,
            "No number in size-in-bytes value '" + input + "'");
    }
    MemoryUnit units = MemoryUnit::parseUnit(unitString);

    if (MemoryUnit::isNull(units)) {
        throw ConfigExceptionBadValue(
            originForException,
            pathForException,
            "Could not parse size-in-bytes unit '" + unitString + "' (try k, K, kB, KiB, kilobytes, kibibytes)");
    }

    try {
        // if the string is purely digits, parse as an integer to avoid
        // possible precision loss; otherwise as a double.
        if (std::all_of(numberString.begin(), numberString.end(), (int(*)(int))std::isdigit)) {
            return boost::lexical_cast<int64_t>(numberString) * units.bytes;
        }
        else {
            return static_cast<int64_t>(boost::lexical_cast<double>(numberString) * units.bytes);
        }
    } catch (boost::bad_lexical_cast&) {
        throw ConfigExceptionBadValue(
            originForException,
            pathForException,
            "Could not parse size-in-bytes number '" + numberString + "'");
    }
}

AbstractConfigValuePtr SimpleConfig::peekPath(const PathPtr& path) {
    return std::dynamic_pointer_cast<AbstractConfigObject>(root())->peekPath(path);
}

void SimpleConfig::addProblem(VectorValidationProblem& accumulator, const PathPtr& path, const ConfigOriginPtr& origin, const std::string& problem) {
    accumulator.push_back(ValidationProblem(path->render(), origin, problem));
}

std::string SimpleConfig::getDesc(const ConfigValuePtr& refValue) {
    if (instanceof<AbstractConfigObject>(refValue)) {
        auto obj = std::dynamic_pointer_cast<AbstractConfigObject>(refValue);
        if (obj->empty()) {
            return "object";
        }
        else {
            std::string keySet;
            for (auto& pair : *obj) {
                if (!keySet.empty()) {
                    keySet += ", ";
                }
                keySet += pair.first;
            }
            return "object with keys " + keySet;
        }
    }
    else if (instanceof<SimpleConfigList>(refValue)) {
        return "list";
    }
    else {
        return boost::to_lower_copy(ConfigValueTypeEnum::name(refValue->valueType()));
    }
}

void SimpleConfig::addMissing(VectorValidationProblem& accumulator, const ConfigValuePtr& refValue, const PathPtr& path, const ConfigOriginPtr& origin) {
    addProblem(accumulator, path, origin, "No setting at '" + path->render() + "', expecting: " + getDesc(refValue));
}

void SimpleConfig::addWrongType(VectorValidationProblem& accumulator, const ConfigValuePtr& refValue, const AbstractConfigValuePtr& actual, const PathPtr& path) {
    addProblem(accumulator, path, actual->origin(), "Wrong value type at '" + path->render() + "', expecting: " + getDesc(refValue) + " but got: " + getDesc(actual));
}

bool SimpleConfig::couldBeNull(const AbstractConfigValuePtr& v) {
    return DefaultTransformer::transform(v, ConfigValueType::NONE)->valueType() == ConfigValueType::NONE;
}

bool SimpleConfig::haveCompatibleTypes(const ConfigValuePtr& reference, const AbstractConfigValuePtr& value) {
    if (couldBeNull(std::dynamic_pointer_cast<AbstractConfigValue>(reference)) || couldBeNull(value)) {
        // we allow any setting to be null
        return true;
    }
    else if (instanceof<AbstractConfigObject>(reference)) {
        if (instanceof<AbstractConfigObject>(value)) {
            return true;
        }
        else {
            return false;
        }
    }
    else if (instanceof<SimpleConfigList>(reference)) {
        if (instanceof<SimpleConfigList>(value)) {
            return true;
        }
        else {
            return false;
        }
    }
    else if (instanceof<ConfigString>(reference)) {
        // assume a string could be gotten as any non-collection type;
        // allows things like getMilliseconds including domain-specific
        // interpretations of strings
        return true;
    }
    else if (instanceof<ConfigString>(value)) {
        // assume a string could be gotten as any non-collection type
        return true;
    }
    else {
        if (reference->valueType() == value->valueType()) {
            return true;
        }
        else {
            return false;
        }
    }
}

void SimpleConfig::checkValidObject(const PathPtr& path, const AbstractConfigObjectPtr& reference, const AbstractConfigObjectPtr& value, VectorValidationProblem& accumulator) {
    for (auto& entry : *reference) {
        std::string key = entry.first;

        PathPtr childPath;
        if (path) {
            childPath = Path::newKey(key)->prepend(path);
        }
        else {
            childPath = Path::newKey(key);
        }

        auto v = value->find(key);
        if (v == value->end()) {
            addMissing(accumulator, entry.second, childPath, value->origin());
        }
        else {
            checkValid(childPath, entry.second, std::dynamic_pointer_cast<AbstractConfigValue>(v->second), accumulator);
        }
    }
}

void SimpleConfig::checkValid(const PathPtr& path, const ConfigValuePtr& reference, const AbstractConfigValuePtr& value, VectorValidationProblem& accumulator) {
    // Unmergeable is supposed to be impossible to encounter in here
    // because we check for resolve status up front.

    if (haveCompatibleTypes(reference, value)) {
        if (instanceof<AbstractConfigObject>(reference) && instanceof<AbstractConfigObject>(value)) {
            checkValidObject(path, std::dynamic_pointer_cast<AbstractConfigObject>(reference), std::dynamic_pointer_cast<AbstractConfigObject>(value), accumulator);
        }
        else if (instanceof<SimpleConfigList>(reference) && instanceof<SimpleConfigList>(value)) {
            auto listRef = std::dynamic_pointer_cast<SimpleConfigList>(reference);
            auto listValue = std::dynamic_pointer_cast<SimpleConfigList>(value);
            if (listRef->empty() || listValue->empty()) {
                // can't verify type, leave alone
            }
            else {
                auto refElement = listRef->at(0);
                for (auto& elem : *listValue) {
                    auto e = std::dynamic_pointer_cast<AbstractConfigValue>(elem);
                    if (!haveCompatibleTypes(refElement, e)) {
                        addProblem(accumulator, path, e->origin(), "List at '" + path->render() + "' contains wrong value type, expecting list of " + getDesc(refElement) + " but got element of type " + getDesc(e));
                        // don't add a problem for every last array element
                        break;
                    }
                }
            }
        }
    }
    else {
        addWrongType(accumulator, reference, value, path);
    }
}

void SimpleConfig::checkValid(const ConfigPtr& reference, const VectorString& restrictToPaths) {
    auto ref = std::dynamic_pointer_cast<SimpleConfig>(reference);

    // unresolved reference config is a bug in the caller of checkValid
    if (std::dynamic_pointer_cast<AbstractConfigValue>(ref->root())->resolveStatus() != ResolveStatus::RESOLVED) {
        throw ConfigExceptionBugOrBroken("do not call checkValid() with an unresolved reference config, call Config#resolve(), see Config#resolve() API docs");
    }

    // unresolved config under validation is a bug in something,
    // NotResolved is a more specific subclass of BugOrBroken
    if (std::dynamic_pointer_cast<AbstractConfigValue>(root())->resolveStatus() != ResolveStatus::RESOLVED) {
        throw ConfigExceptionNotResolved("need to Config#resolve() each config before using it, see the API docs for Config#resolve()");
    }

    // Now we know that both reference and this config are resolved

    VectorValidationProblem problems;

    if (restrictToPaths.empty()) {
        checkValidObject(nullptr, std::dynamic_pointer_cast<AbstractConfigObject>(ref->root()), std::dynamic_pointer_cast<AbstractConfigObject>(root()), problems);
    }
    else {
        for (std::string p : restrictToPaths) {
            auto path = Path::newPath(p);
            auto refValue = ref->peekPath(path);
            if (refValue) {
                auto child = peekPath(path);
                if (child) {
                    checkValid(path, refValue, child, problems);
                }
                else {
                    addMissing(problems, refValue, path, origin());
                }
            }
        }
    }

    if (!problems.empty()) {
        throw ConfigExceptionValidationFailed(problems);
    }
}

ConfigPtr SimpleConfig::withOnlyPath(const std::string& pathExpression) {
    auto path = Path::newPath(pathExpression);
    return make_instance(std::dynamic_pointer_cast<AbstractConfigObject>(root())->withOnlyPath(path));
}

ConfigPtr SimpleConfig::withoutPath(const std::string& pathExpression) {
    auto path = Path::newPath(pathExpression);
    return make_instance(std::dynamic_pointer_cast<AbstractConfigObject>(root())->withoutPath(path));
}

}
