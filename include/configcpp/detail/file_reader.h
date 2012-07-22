/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef FILE_READER_H_
#define FILE_READER_H_

#include "configcpp/detail/reader.h"
#include "configcpp/detail/config_base.h"

namespace config {

///
/// Convenience class for reading character files.
///
class FileReader : public virtual Reader, public ConfigBase {
public:
    CONFIG_CLASS(FileReader);

    FileReader(const std::string& file);

    virtual int32_t read() override;
    virtual void close() override;

private:
    bool fillBuffer();

private:
    static const uint32_t BUFFER_SIZE = 8096;

    std::ifstream file;

    char buffer[BUFFER_SIZE];
    uint32_t size;
    uint32_t position;
};

}

#endif // FILE_READER_H_
