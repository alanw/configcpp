/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/path.h"
#include "configcpp/detail/path_builder.h"
#include "configcpp/detail/config_impl_util.h"
#include "configcpp/detail/parser.h"
#include "configcpp/detail/variant_utils.h"
#include "configcpp/config_exception.h"

namespace config {

Path::Path(const std::string& first, const PathPtr& remainder) :
    first_(first),
    remainder_(remainder) {
}

Path::Path(const VectorString& elements) {
    if (elements.empty()) {
        throw ConfigExceptionBugOrBroken("empty path");
    }
    first_ = elements[0];
    if (elements.size() > 1) {
        auto pb = PathBuilder::make_instance();
        for (uint32_t i = 1; i < elements.size(); ++i) {
            pb->appendKey(elements[i]);
        }
        remainder_ = pb->result();
    }
}

Path::Path(const VectorPath& pathsToConcat) {
    if (pathsToConcat.empty()) {
        throw ConfigExceptionBugOrBroken("empty path");
    }

    auto i = pathsToConcat.begin();
    auto firstPath = *(i++);
    first_ = firstPath->first_;

    auto pb = PathBuilder::make_instance();
    if (firstPath->remainder_) {
        pb->appendPath(firstPath->remainder_);
    }
    while (i != pathsToConcat.end()) {
        pb->appendPath(*(i++));
    }
    remainder_ = pb->result();
}

std::string Path::first() {
    return first_;
}

PathPtr Path::remainder() {
    return remainder_;
}

PathPtr Path::parent() {
    if (!remainder_) {
        return nullptr;
    }

    auto pb = PathBuilder::make_instance();
    auto p = shared_from_this();
    while (p->remainder_) {
        pb->appendKey(p->first_);
        p = p->remainder_;
    }
    return pb->result();
}

std::string Path::last() {
    auto p = shared_from_this();
    while (p->remainder_) {
        p = p->remainder_;
    }
    return p->first_;
}

PathPtr Path::prepend(const PathPtr& toPrepend) {
    auto pb = PathBuilder::make_instance();
    pb->appendPath(toPrepend);
    pb->appendPath(shared_from_this());
    return pb->result();
}

uint32_t Path::length() {
    uint32_t count = 1;
    auto p = remainder_;
    while (p) {
        count += 1;
        p = p->remainder_;
    }
    return count;
}

PathPtr Path::subPath(uint32_t removeFromFront) {
    int32_t count = removeFromFront;
    auto p = shared_from_this();
    while (p && count > 0) {
        count -= 1;
        p = p->remainder_;
    }
    return p;
}

PathPtr Path::subPath(uint32_t firstIndex, uint32_t lastIndex) {
    if (lastIndex < firstIndex) {
        throw ConfigExceptionBugOrBroken("bad call to subPath");
    }

    auto from = subPath(firstIndex);
    auto pb = PathBuilder::make_instance();
    uint32_t count = lastIndex - firstIndex;
    while (count > 0) {
        count -= 1;
        pb->appendKey(from->first());
        from = from->remainder();
        if (!from) {
            throw ConfigExceptionBugOrBroken("subPath lastIndex out of range " + boost::lexical_cast<std::string>(lastIndex));
        }
    }
    return pb->result();
}

bool Path::equals(const ConfigVariant& other) {
    if (instanceof<Path>(other)) {
        auto that = static_get<Path>(other);
        return this->first_ == that->first_ && ConfigImplUtil::equalsHandlingNull(this->remainder_, that->remainder_);
    }
    else {
        return false;
    }
}

uint32_t Path::hashCode() {
    return 41 * (41 + std::hash<std::string>()(first_)) + (!remainder_ ? 0 : remainder_->hashCode());
}

bool Path::hasFunkyChars(const std::string& s) {
    uint32_t length = s.length();

    if (length == 0) {
        return false;
    }

    // if the path starts with something that could be a number,
    // we need to quote it because the number could be invalid,
    // for example it could be a hyphen with no digit afterward
    // or the exponent "e" notation could be mangled.
    char first = s[0];
    if (!std::isalpha(first)) {
        return true;
    }

    for (uint32_t i = 1; i < length; ++i) {
        char c = s[i];

        if (std::isalnum(c) || c == '-' || c == '_') {
            continue;
        }
        else {
            return true;
        }
    }
    return false;
}

void Path::appendToStream(std::string& s) {
    if (hasFunkyChars(first_) || first_.empty()) {
        s += ConfigImplUtil::renderJsonString(first_);
    }
    else {
        s += first_;
    }
    if (remainder_) {
        s += ".";
        remainder_->appendToStream(s);
    }
}

std::string Path::toString() {
    std::string s = "Path(";
    appendToStream(s);
    s += ")";
    return s;
}

std::string Path::render() {
    std::string s;
    appendToStream(s);
    return s;
}

PathPtr Path::newKey(const std::string& key) {
    return make_instance(key, nullptr);
}

PathPtr Path::newPath(const std::string& path) {
    return Parser::parsePath(path);
}

}
