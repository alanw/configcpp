/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef SIMPLE_CONFIG_LIST_H_
#define SIMPLE_CONFIG_LIST_H_

#include "configcpp/detail/abstract_config_value.h"
#include "configcpp/config_list.h"

namespace config {

class SimpleConfigList : public AbstractConfigValue, public virtual ConfigList {
public:
    CONFIG_CLASS(SimpleConfigList);

    SimpleConfigList(const ConfigOriginPtr& origin,
                     const VectorAbstractConfigValue& value);
    SimpleConfigList(const ConfigOriginPtr& origin,
                     const VectorAbstractConfigValue& value,
                     ResolveStatus status);

    virtual ConfigValueType valueType() override;
    virtual ConfigVariant unwrapped() override;

    virtual ResolveStatus resolveStatus() override;

private:
    SimpleConfigListPtr modify(const NoExceptionsModifierPtr& modifier,
                               ResolveStatus newResolveStatus);
    SimpleConfigListPtr modifyMayThrow(const ModifierPtr& modifier,
                                       ResolveStatus newResolveStatus);

public:
    virtual AbstractConfigValuePtr resolveSubstitutions(const ResolveContextPtr& context) override;

    virtual AbstractConfigValuePtr relativized(const PathPtr& prefix) override;

protected:
    virtual bool canEqual(const ConfigVariant& other) override;

public:
    virtual bool equals(const ConfigVariant& other) override;
    virtual uint32_t hashCode() override;

protected:
    virtual void render(std::string& s,
                        uint32_t indent,
                        const ConfigRenderOptionsPtr& options) override;

public:
    virtual VectorConfigValue::const_iterator begin() const override;
    virtual VectorConfigValue::const_iterator end() const override;
    virtual VectorConfigValue::const_reference at(VectorConfigValue::size_type n) const override;
    virtual VectorConfigValue::const_reference front() const override;
    virtual VectorConfigValue::const_reference back() const override;
    virtual VectorConfigValue::const_reference operator[](VectorConfigValue::size_type n) const override;
    virtual bool empty() const override;
    virtual VectorConfigValue::size_type size() const override;
    virtual void clear() override;
    virtual void pop_back() override;
    virtual void resize(VectorConfigValue::size_type n,
                        const VectorConfigValue::value_type& val = nullptr) override;
    virtual VectorConfigValue::const_iterator erase(VectorConfigValue::const_iterator pos) override;
    virtual VectorConfigValue::const_iterator insert(VectorConfigValue::const_iterator pos,
                                                     const VectorConfigValue::value_type& val) override;

private:
    static ConfigExceptionUnsupportedOperation weAreImmutable(const std::string& method);

public:
    virtual AbstractConfigValuePtr newCopy(const ConfigOriginPtr& origin) override;

    SimpleConfigListPtr concatenate(const SimpleConfigListPtr& other);

private:
    VectorConfigValue value;
    bool resolved;
};

class SimpleConfigListModifier : public virtual Modifier, public ConfigBase {
public:
    CONFIG_CLASS(SimpleConfigListModifier);

    SimpleConfigListModifier(const ResolveContextPtr& context);

    virtual AbstractConfigValuePtr modifyChildMayThrow(const std::string& keyOrNull,
                                                       const AbstractConfigValuePtr& v) override;

private:
    ResolveContextPtr context;
};

class SimpleConfigListNoExceptionsModifier : public NoExceptionsModifier {
public:
    CONFIG_CLASS(SimpleConfigListNoExceptionsModifier);

    SimpleConfigListNoExceptionsModifier(const PathPtr& prefix);

    virtual AbstractConfigValuePtr modifyChild(const std::string& keyOrNull,
                                               const AbstractConfigValuePtr& v) override;

private:
    PathPtr prefix;
};

}

#endif // SIMPLE_CONFIG_LIST_H_
