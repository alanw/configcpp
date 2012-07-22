/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "test_fixture.h"
#include "configcpp/detail/string_reader.h"

using namespace config;

class StringReaderTest : public TestFixture {
};

TEST_F(StringReaderTest, readChars) {
    auto reader = StringReader::make_instance("hello world");
    std::string output;
    for (int32_t read = reader->read(); read != Reader::READER_EOF; read = reader->read()) {
        output += static_cast<char>(read);
    }
    EXPECT_EQ("hello world", output);
}

TEST_F(StringReaderTest, readPastEof) {
    auto reader = StringReader::make_instance("hello world");
    while (reader->read() != Reader::READER_EOF) {
    }
    EXPECT_EQ(static_cast<int32_t>(Reader::READER_EOF), reader->read());
    EXPECT_EQ(static_cast<int32_t>(Reader::READER_EOF), reader->read());
}

TEST_F(StringReaderTest, readAfterClosing) {
    auto reader = StringReader::make_instance("hello world");
    reader->read();
    reader->read();
    reader->read();
    reader->read();
    reader->close();
    EXPECT_EQ(static_cast<int32_t>(Reader::READER_EOF), reader->read());
    EXPECT_EQ(static_cast<int32_t>(Reader::READER_EOF), reader->read());
}
