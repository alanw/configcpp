/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef READER_H_
#define READER_H_

#include "configcpp/config_types.h"

namespace config {

///
/// Abstract class for reading character streams.
///
class Reader {
public:
    static const int32_t READER_EOF = -1;

    /// Read a single character.
    virtual int32_t read() = 0;

    /// Close the stream.
    virtual void close() = 0;
};

}

#endif // READER_H_
