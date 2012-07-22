/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_REFERENCE_H_
#define CONFIG_REFERENCE_H_

#include "configcpp/detail/abstract_config_value.h"
#include "configcpp/detail/unmergeable.h"

namespace config {

///
/// ConfigReference replaces ConfigReference (the older class kept for back
/// compat) and represents the ${} substitution syntax. It can resolve to any
/// kind of value.
///
class ConfigReference : public AbstractConfigValue, public virtual Unmergeable {
public:
    CONFIG_CLASS(ConfigReference);

    ConfigReference(const ConfigOriginPtr& origin,
                    const SubstitutionExpressionPtr& expr);
    ConfigReference(const ConfigOriginPtr& origin,
                    const SubstitutionExpressionPtr& expr,
                    uint32_t prefixLength);

public:
    virtual ConfigValueType valueType() override;
    virtual ConfigVariant unwrapped() override;

protected:
    virtual AbstractConfigValuePtr newCopy(const ConfigOriginPtr& origin) override;

public:
    virtual bool ignoresFallbacks() override;

    virtual VectorAbstractConfigValue unmergedValues() override;

    /// ConfigReference should be a firewall against NotPossibleToResolve going
    /// further up the stack; it should convert everything to ConfigException.
    /// This way it's impossible for NotPossibleToResolve to "escape" since
    /// any failure to resolve has to start with a ConfigReference.
    virtual AbstractConfigValuePtr resolveSubstitutions(const ResolveContextPtr& context) override;

    virtual ResolveStatus resolveStatus() override;

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
    SubstitutionExpressionPtr expression();

private:
    SubstitutionExpressionPtr expr;

    // the length of any prefixes added with relativized()
    uint32_t prefixLength;
};

}

#endif // CONFIG_REFERENCE_H_
