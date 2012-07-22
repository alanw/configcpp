/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef SIMPLE_CONFIG_ORIGIN_H_
#define SIMPLE_CONFIG_ORIGIN_H_

#include "configcpp/detail/config_base.h"
#include "configcpp/config_origin.h"

namespace config {

///
/// It would be cleaner to have a class hierarchy for various origin types,
/// but was hoping this would be enough simpler to be a little messy. eh.
///
class SimpleConfigOrigin : public virtual ConfigOrigin, public ConfigBase {
public:
    CONFIG_CLASS(SimpleConfigOrigin);

    SimpleConfigOrigin(const std::string& description,
                       int32_t lineNumber,
                       int32_t endLineNumber,
                       OriginType originType,
                       const VectorString& commentsOrNull);

public:
    static SimpleConfigOriginPtr newSimple(const std::string& description);
    static SimpleConfigOriginPtr newFile(const std::string& filename);

    SimpleConfigOriginPtr setLineNumber(int32_t lineNumber);
    SimpleConfigOriginPtr setComments(const VectorString& comments);

    virtual std::string description() override;

    virtual bool equals(const ConfigVariant& other) override;
    virtual uint32_t hashCode() override;
    virtual std::string toString() override;

    virtual std::string filename() override;
    virtual int32_t lineNumber() override;
    virtual VectorString comments() override;

private:
    static SimpleConfigOriginPtr mergeTwo(const SimpleConfigOriginPtr& a,
                                          const SimpleConfigOriginPtr& b);
    static uint32_t similarity(const SimpleConfigOriginPtr& a,
                               const SimpleConfigOriginPtr& b);
    static SimpleConfigOriginPtr mergeThree(const SimpleConfigOriginPtr& a,
                                            const SimpleConfigOriginPtr& b,
                                            const SimpleConfigOriginPtr& c);

public:
    static ConfigOriginPtr mergeOrigins(const ConfigOriginPtr& a,
                                        const ConfigOriginPtr& b);
    static ConfigOriginPtr mergeOrigins(const VectorAbstractConfigValue& stack);
    static ConfigOriginPtr mergeOrigins(const VectorConfigOrigin& stack);

private:
    std::string description_;
    int32_t lineNumber_;
    int32_t endLineNumber;
    OriginType originType;
    VectorString commentsOrNull;
};

}

#endif // SIMPLE_CONFIG_ORIGIN_H_
