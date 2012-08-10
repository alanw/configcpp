/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_CONCATENATION_H_
#define CONFIG_CONCATENATION_H_

#include "configcpp/detail/abstract_config_value.h"
#include "configcpp/detail/unmergeable.h"

namespace config {

///
/// A ConfigConcatenation represents a list of values to be concatenated (see the
/// spec). It only has to exist if at least one value is an unresolved
/// substitution, otherwise we could go ahead and collapse the list into a single
/// value.
///
/// Right now this is always a list of strings and ${} references, but in the
/// future should support a list of ConfigList. We may also support
/// concatenations of objects, but ConfigDelayedMerge should be used for that
/// since a concat of objects really will merge, not concatenate.
///
class ConfigConcatenation : public AbstractConfigValue, public virtual Unmergeable {
public:
    CONFIG_CLASS(ConfigConcatenation);

    ConfigConcatenation(const ConfigOriginPtr& origin, const VectorAbstractConfigValue& pieces);

private:
    ConfigExceptionNotResolved notResolved();

public:
    virtual ConfigValueType valueType() override;
    virtual ConfigVariant unwrapped() override;

protected:
    virtual AbstractConfigValuePtr newCopy(const ConfigOriginPtr& origin) override;

public:
    virtual bool ignoresFallbacks() override;

    virtual VectorAbstractConfigValue unmergedValues() override;

    static void join(VectorAbstractConfigValue& builder, const AbstractConfigValuePtr& right);
    static VectorAbstractConfigValue consolidate(const VectorAbstractConfigValue& pieces);
    static AbstractConfigValuePtr concatenate(const VectorAbstractConfigValue& pieces);

    virtual AbstractConfigValuePtr resolveSubstitutions(const ResolveContextPtr& context) override;

    virtual ResolveStatus resolveStatus() override;

    /// When you graft a substitution into another object, you have to prefix
    /// it with the location in that object where you grafted it; but save
    /// prefixLength so system property and env variable lookups don't get
    /// broken.
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

private:
    VectorAbstractConfigValue pieces;
};

}

#endif // CONFIG_CONCATENATION_H_
