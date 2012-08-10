/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "test_fixture.h"
#include "configcpp/detail/file_reader.h"
#include "configcpp/config_exception.h"

using namespace config;

class FileReaderTest : public TestFixture {
};

TEST_F(FileReaderTest, readChars) {
    std::string filePath = resourcePath() + "/lorem_ipsum";
    auto reader = FileReader::make_instance(filePath);
    std::string output;
    for (int32_t read = reader->read(); read != Reader::READER_EOF; read = reader->read()) {
        output += static_cast<char>(read);
    }
    EXPECT_TRUE(boost::starts_with(output, "Lorem ipsum"));
    EXPECT_TRUE(boost::ends_with(output, "adipiscing elit."));
    VectorString lines;
    boost::split(lines, output, boost::is_any_of("\n"));
    EXPECT_EQ(1000, lines.size());
    for (auto& line : lines) {
        EXPECT_EQ("Lorem ipsum dolor sit amet, consectetur adipiscing elit.", line);
    }
}

TEST_F(FileReaderTest, readPastEof) {
    std::string filePath = resourcePath() + "/lorem_ipsum";
    auto reader = FileReader::make_instance(filePath);
    while (reader->read() != Reader::READER_EOF) {
    }
    EXPECT_EQ(static_cast<int32_t>(Reader::READER_EOF), reader->read());
    EXPECT_EQ(static_cast<int32_t>(Reader::READER_EOF), reader->read());
}

TEST_F(FileReaderTest, readAfterClosing) {
    std::string filePath = resourcePath() + "/lorem_ipsum";
    auto reader = FileReader::make_instance(filePath);
    reader->read();
    reader->read();
    reader->read();
    reader->read();
    reader->close();
    EXPECT_EQ(static_cast<int32_t>(Reader::READER_EOF), reader->read());
    EXPECT_EQ(static_cast<int32_t>(Reader::READER_EOF), reader->read());
}

TEST_F(FileReaderTest, badPath) {
    std::string filePath = "bad path";
    EXPECT_THROW(FileReader::make_instance(filePath), ConfigExceptionFileNotFound);
}

TEST_F(FileReaderTest, fileNotExist) {
    std::string filePath = resourcePath() + "/not_exist";
    EXPECT_THROW(FileReader::make_instance(filePath), ConfigExceptionFileNotFound);
}
