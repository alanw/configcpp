/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef SUBSTITUTION_EXPRESSION_H_
#define SUBSTITUTION_EXPRESSION_H_

#include "configcpp/detail/config_base.h"

namespace config {

class SubstitutionExpression : public ConfigBase {
public:
    CONFIG_CLASS(SubstitutionExpression);

    SubstitutionExpression(const PathPtr& path, bool optional);

    PathPtr path();
    bool optional();

    SubstitutionExpressionPtr changePath(const PathPtr& newPath);

    virtual std::string toString() override;
    virtual bool equals(const ConfigVariant& other) override;
    virtual uint32_t hashCode() override;

private:
    PathPtr path_;
    bool optional_;
};

}

#endif // SUBSTITUTION_EXPRESSION_H_
