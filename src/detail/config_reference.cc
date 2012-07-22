/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/config_reference.h"
#include "configcpp/detail/substitution_expression.h"
#include "configcpp/detail/resolve_replacer.h"
#include "configcpp/detail/resolve_context.h"
#include "configcpp/detail/resolve_source.h"
#include "configcpp/detail/abstract_config_value.h"
#include "configcpp/detail/path.h"
#include "configcpp/config_exception.h"
#include "configcpp/config_value_type.h"

namespace config {

ConfigReference::ConfigReference(const ConfigOriginPtr& origin, const SubstitutionExpressionPtr& expr) :
    ConfigReference(origin, expr, 0) {
}

ConfigReference::ConfigReference(const ConfigOriginPtr& origin, const SubstitutionExpressionPtr& expr, uint32_t prefixLength) :
    AbstractConfigValue(origin),
    expr(expr),
    prefixLength(prefixLength) {
}

ConfigValueType ConfigReference::valueType() {
    throw ConfigExceptionNotResolved("need to Config#resolve(), see the API docs for Config#resolve(); substitution not resolved: " + ConfigBase::toString());
}

ConfigVariant ConfigReference::unwrapped() {
    throw ConfigExceptionNotResolved("need to Config#resolve(), see the API docs for Config#resolve(); substitution not resolved: " + ConfigBase::toString());
}

AbstractConfigValuePtr ConfigReference::newCopy(const ConfigOriginPtr& origin) {
    return make_instance(origin, expr, prefixLength);
}

bool ConfigReference::ignoresFallbacks() {
    return false;
}

VectorAbstractConfigValue ConfigReference::unmergedValues() {
    return {shared_from_this()};
}

AbstractConfigValuePtr ConfigReference::resolveSubstitutions(const ResolveContextPtr& context) {
    context->source()->replace(shared_from_this(), ResolveReplacer::cycleResolveReplacer());
    ConfigExceptionPtr finally;
    AbstractConfigValuePtr v;
    try {
        try {
            v = context->source()->lookupSubst(context, expr, prefixLength);
        }
        catch (NotPossibleToResolve& e) {
            if (expr->optional()) {
                v.reset();
            }
            else {
                throw ConfigExceptionUnresolvedSubstitution(
                    origin(),
                    expr->toString() + " was part of a cycle of substitutions involving " + e.traceString());
            }
        }

        if (!v && !expr->optional()) {
            throw ConfigExceptionUnresolvedSubstitution(origin(), expr->toString());
        }

    }
    catch (ConfigException& e) {
        finally = e.clone();
    }
    context->source()->unreplace(shared_from_this());

    if (finally) {
        finally->raise();
    }

    return v;
}

ResolveStatus ConfigReference::resolveStatus() {
    return ResolveStatus::UNRESOLVED;
}

AbstractConfigValuePtr ConfigReference::relativized(const PathPtr& prefix) {
    auto newExpr = expr->changePath(expr->path()->prepend(prefix));
    return make_instance(origin(), newExpr, prefixLength + prefix->length());
}

bool ConfigReference::canEqual(const ConfigVariant& other) {
    return instanceof<ConfigReference>(other);
}

bool ConfigReference::equals(const ConfigVariant& other) {
    // note that "origin" is deliberately NOT part of equality
    if (instanceof<ConfigReference>(other)) {
        return canEqual(other) && this->expr->equals(static_get<ConfigReference>(other)->expr);
    }
    else {
        return false;
    }
}

uint32_t ConfigReference::hashCode() {
    // note that "origin" is deliberately NOT part of equality
    return expr->hashCode();
}

void ConfigReference::render(std::string& s, uint32_t indent, const ConfigRenderOptionsPtr& options) {
    s += expr->toString();
}

SubstitutionExpressionPtr ConfigReference::expression() {
    return expr;
}

}
