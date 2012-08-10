/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/substitution_expression.h"
#include "configcpp/detail/path.h"
#include "configcpp/detail/variant_utils.h"

namespace config {

SubstitutionExpression::SubstitutionExpression(const PathPtr& path, bool optional) :
    path_(path),
    optional_(optional) {
}

PathPtr SubstitutionExpression::path() {
    return path_;
}

bool SubstitutionExpression::optional() {
    return optional_;
}

SubstitutionExpressionPtr SubstitutionExpression::changePath(const PathPtr& newPath) {
    if (newPath == path_) {
        return shared_from_this();
    }
    else {
        return make_instance(newPath, optional_);
    }
}

std::string SubstitutionExpression::toString() {
    return std::string("${") + (optional_ ? "?" : "") + path_->render() + "}";
}

bool SubstitutionExpression::equals(const ConfigVariant& other) {
    if (instanceof<SubstitutionExpression>(other)) {
        auto otherExp = static_get<SubstitutionExpression>(other);
        return otherExp->path_->equals(this->path_) && otherExp->optional_ == this->optional_;
    }
    else {
        return false;
    }
}

uint32_t SubstitutionExpression::hashCode() {
    uint32_t hash = 41 * (41 + path_->hashCode());
    hash = 41 * (hash + (optional_ ? 1 : 0));
    return hash;
}

}
