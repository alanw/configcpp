/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_BASE_H_
#define CONFIG_BASE_H_

#include "configcpp/detail/instance_utils.h"
#include "configcpp/detail/misc_utils.h"
#include "configcpp/config_types.h"

#define CONFIG_CLASS(Name) \
    std::shared_ptr<Name> shared_from_this() { \
        return std::static_pointer_cast<Name>(ConfigBase::shared_from_this()); \
    } \
    template <class... Args> \
    static std::shared_ptr<Name> make_instance(Args&& ... args) { \
        std::shared_ptr<Name> instance = std::make_shared<Name>(args...); \
        instance->initialize(); \
        return instance; \
    } \
    virtual std::string getClassName() override { return #Name; }

namespace config {

///
/// Base class of all config object types. Implements common methods that
/// emulate Java objects.
///
/// <p>
/// You instantiate a config object by calling make_instance(). This creates a
/// a std::shared_ptr to the new object then calls an initialiser. This is
/// useflu in some circumstances as you may want to use shared_from_this() in
/// the constructor, which isn't allowed:
///
/// <pre>
///     auto config = Config::make_instance();
///     config->foo();
/// </pre>
///
/// Note: You must define the {@code CONFIG_CLASS} macro in classes derived from
/// ConfigBase.
///
class ConfigBase : public std::enable_shared_from_this<ConfigBase> {
public:
    virtual ~ConfigBase();

    /// Called directly after instantiation to create objects that depend on
    /// this object being fully constructed.
    virtual void initialize();

    /// Return hash code for this object.
    virtual uint32_t hashCode();

    /// Return whether two objects are equal.
    virtual bool equals(const ConfigVariant& other);

    /// Returns a string representation of the object.
    virtual std::string toString();

    /// Return the class name of the object.
    virtual std::string getClassName();
};

}

#endif // CONFIG_BASE_H_
