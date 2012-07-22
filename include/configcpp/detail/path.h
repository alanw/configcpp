/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef PATH_H_
#define PATH_H_

#include "configcpp/detail/config_base.h"

namespace config {

class Path : public ConfigBase {
public:
    CONFIG_CLASS(Path);

    Path(const std::string& first, const PathPtr& remainder);
    Path(const VectorString& elements = VectorString());
    Path(const VectorPath& pathsToConcat);

    std::string first();

    /// @return path minus the first element or null if no more elements
    PathPtr remainder();

    /// @return path minus the last element or null if we have just one element
    PathPtr parent();

    /// @return last element in the path
    std::string last();

    PathPtr prepend(const PathPtr& toPrepend);

    uint32_t length();

    PathPtr subPath(uint32_t removeFromFront);
    PathPtr subPath(uint32_t firstIndex, uint32_t lastIndex);

    virtual bool equals(const ConfigVariant& other) override;
    virtual uint32_t hashCode() override;

    /// This doesn't have a very precise meaning, just to reduce
    /// noise from quotes in the rendered path for average cases.
    static bool hasFunkyChars(const std::string& s);

private:
    void appendToStream(std::string& s);

public:
    virtual std::string toString() override;

    /// toString() is a debugging-oriented version while this is an
    /// error-message-oriented human-readable one.
    std::string render();

    static PathPtr newKey(const std::string& key);
    static PathPtr newPath(const std::string& path);

private:
    std::string first_;
    PathPtr remainder_;
};

}

#endif // PATH_H_
