/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef STRING_READER_H_
#define STRING_READER_H_

#include "configcpp/detail/reader.h"
#include "configcpp/detail/config_base.h"

namespace config {

///
/// A character stream whose source is a string.
///
class StringReader : public virtual Reader, public ConfigBase {
public:
    CONFIG_CLASS(StringReader);

    StringReader(const std::string& str);

    virtual int32_t read() override;
    virtual void close() override;

private:
    std::string str;
    uint32_t position;
};

}

#endif // STRING_READER_H_
