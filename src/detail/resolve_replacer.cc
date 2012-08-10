/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/resolve_replacer.h"
#include "configcpp/detail/abstract_config_value.h"

namespace config {

AbstractConfigValuePtr ResolveReplacer::replace(const ResolveContextPtr& context) {
    if (!replacement) {
        replacement = makeReplacement(context);
    }
    return replacement;
}

ResolveReplacerPtr ResolveReplacer::cycleResolveReplacer() {
    return ResolveReplacerCycle::make_instance();
}

AbstractConfigValuePtr ResolveReplacerCycle::makeReplacement(const ResolveContextPtr& context) {
    throw NotPossibleToResolve(context);
}

}
