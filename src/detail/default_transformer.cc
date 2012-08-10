/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/default_transformer.h"
#include "configcpp/detail/config_int64.h"
#include "configcpp/detail/config_double.h"
#include "configcpp/detail/config_null.h"
#include "configcpp/detail/config_boolean.h"
#include "configcpp/detail/config_string.h"
#include "configcpp/config_value_type.h"

namespace config {

AbstractConfigValuePtr DefaultTransformer::transform(const AbstractConfigValuePtr& value, ConfigValueType requested) {
    if (value->valueType() == ConfigValueType::STRING) {
        std::string s = value->unwrapped<std::string>();
        switch (requested) {
            case ConfigValueType::NUMBER:
                try {
                    int64_t v = boost::lexical_cast<int64_t>(s);
                    return ConfigInt64::make_instance(value->origin(), v, s);
                }
                catch (boost::bad_lexical_cast&) {
                    // try double
                }
                try {
                    double v = boost::lexical_cast<double>(s);
                    return ConfigDouble::make_instance(value->origin(), v, s);
                }
                catch (boost::bad_lexical_cast&) {
                    // oh well
                }
                break;
            case ConfigValueType::NONE:
                if (s == "null") {
                    return ConfigNull::make_instance(value->origin());
                }
                break;
            case ConfigValueType::BOOLEAN:
                if (s == "true" || s == "yes" || s == "on") {
                    return ConfigBoolean::make_instance(value->origin(), true);
                }
                else if (s == "false" || s == "no" || s == "off") {
                    return ConfigBoolean::make_instance(value->origin(), false);
                }
                break;
            case ConfigValueType::LIST:
                // can't go STRING to LIST automatically
                break;
            case ConfigValueType::OBJECT:
                // can't go STRING to OBJECT automatically
                break;
            case ConfigValueType::STRING:
                // no-op STRING to STRING
                break;
        }
    }
    else if (requested == ConfigValueType::STRING) {
        // if we converted null to string here, then you wouldn't properly
        // get a missing-value error if you tried to get a null value
        // as a string.
        switch (value->valueType()) {
            case ConfigValueType::NUMBER: // FALL THROUGH
            case ConfigValueType::BOOLEAN:
                return ConfigString::make_instance(value->origin(), value->transformToString());
            case ConfigValueType::NONE:
                // want to be sure this throws instead of returning "null" as a
                // string
                break;
            case ConfigValueType::OBJECT:
                // no OBJECT to STRING automatically
                break;
            case ConfigValueType::LIST:
                // no LIST to STRING automatically
                break;
            case ConfigValueType::STRING:
                // no-op STRING to STRING
                break;
        }
    }

    return value;
}

}
