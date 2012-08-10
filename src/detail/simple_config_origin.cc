/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/simple_config_origin.h"
#include "configcpp/detail/origin_type.h"
#include "configcpp/detail/abstract_config_value.h"
#include "configcpp/config_exception.h"

namespace config {

SimpleConfigOrigin::SimpleConfigOrigin(const std::string& description,
                                       int32_t lineNumber,
                                       int32_t endLineNumber,
                                       OriginType originType,
                                       const VectorString& commentsOrNull) :
    description_(description),
    lineNumber_(lineNumber),
    endLineNumber(endLineNumber),
    originType(originType),
    commentsOrNull(commentsOrNull) {
    if (description.empty()) {
        throw ConfigExceptionBugOrBroken("description may not be empty");
    }
}

SimpleConfigOriginPtr SimpleConfigOrigin::newSimple(const std::string& description) {
    return SimpleConfigOrigin::make_instance(description, -1, -1, OriginType::GENERIC, VectorString());
}

SimpleConfigOriginPtr SimpleConfigOrigin::newFile(const std::string& filename) {
    return SimpleConfigOrigin::make_instance(filename, -1, -1, OriginType::FILE, VectorString());
}

SimpleConfigOriginPtr SimpleConfigOrigin::setLineNumber(int32_t lineNumber) {
    if (lineNumber == this->lineNumber_ && lineNumber == this->endLineNumber) {
        return shared_from_this();
    }
    else {
        return SimpleConfigOrigin::make_instance(this->description_, lineNumber, lineNumber,
                                                 this->originType, this->commentsOrNull);
    }
}

SimpleConfigOriginPtr SimpleConfigOrigin::setComments(const VectorString& comments) {
    if (comments.size() == commentsOrNull.size() && std::equal(comments.begin(), comments.end(), commentsOrNull.begin())) {
        return shared_from_this();
    }
    else {
        return SimpleConfigOrigin::make_instance(description_, lineNumber_, endLineNumber, originType, comments);
    }
}

std::string SimpleConfigOrigin::description() {
    if (lineNumber_ < 0) {
        return description_;
    }
    else if (endLineNumber == lineNumber_) {
        return description_ + ": " + boost::lexical_cast<std::string>(lineNumber_);
    }
    else {
        return description_ + ": " + boost::lexical_cast<std::string>(lineNumber_) + "-" + boost::lexical_cast<std::string>(endLineNumber);
    }
}

bool SimpleConfigOrigin::equals(const ConfigVariant& other) {
    if (instanceof<SimpleConfigOrigin>(other)) {
        auto otherOrigin = dynamic_get<SimpleConfigOrigin>(other);
        return description_ == otherOrigin->description_ &&
               lineNumber_ == otherOrigin->lineNumber_ &&
               endLineNumber == otherOrigin->endLineNumber &&
               originType == otherOrigin->originType;
    }
    else {
        return false;
    }
}

uint32_t SimpleConfigOrigin::hashCode() {
    uint32_t hash = 41 * (41 + std::hash<std::string>()(description_));
    hash = 41 * (hash + lineNumber_);
    hash = 41 * (hash + endLineNumber);
    hash = 41 * (hash + std::hash<uint32_t>()(static_cast<uint32_t>(originType)));
    return hash;
}

std::string SimpleConfigOrigin::toString() {
    return "ConfigOrigin(" + description_ + ")";
}

std::string SimpleConfigOrigin::filename() {
    if (originType == OriginType::FILE) {
        return description_;
    }
    else {
        return "";
    }
}

int32_t SimpleConfigOrigin::lineNumber() {
    return lineNumber_;
}

VectorString SimpleConfigOrigin::comments() {
    return commentsOrNull;
}

SimpleConfigOriginPtr SimpleConfigOrigin::mergeTwo(const SimpleConfigOriginPtr& a, const SimpleConfigOriginPtr& b) {
    static const std::string MERGE_OF_PREFIX = "merge of ";

    std::string mergedDesc;
    int32_t mergedStartLine = 0;
    int32_t mergedEndLine = 0;
    VectorString mergedComments;

    OriginType mergedType;
    if (a->originType == b->originType) {
        mergedType = a->originType;
    }
    else {
        mergedType = OriginType::GENERIC;
    }

    // first use the "description" field which has no line numbers
    // cluttering it.
    std::string aDesc = a->description_;
    std::string bDesc = b->description_;
    if (boost::starts_with(aDesc, MERGE_OF_PREFIX)) {
        aDesc = aDesc.substr(MERGE_OF_PREFIX.length());
    }
    if (boost::starts_with(bDesc, MERGE_OF_PREFIX)) {
        bDesc = bDesc.substr(MERGE_OF_PREFIX.length());
    }

    if (aDesc == bDesc) {
        mergedDesc = aDesc;

        if (a->lineNumber_ < 0) {
            mergedStartLine = b->lineNumber_;
        }
        else if (b->lineNumber_ < 0) {
            mergedStartLine = a->lineNumber_;
        }
        else {
            mergedStartLine = std::min(a->lineNumber_, b->lineNumber_);
        }

        mergedEndLine = std::max(a->endLineNumber, b->endLineNumber);
    }
    else {
        // this whole merge song-and-dance was intended to avoid this case
        // whenever possible, but we've lost. Now we have to lose some
        // structured information and cram into a string.

        // description() method includes line numbers, so use it instead
        // of description field.
        std::string aFull = a->description();
        std::string bFull = b->description();
        if (boost::starts_with(aFull, MERGE_OF_PREFIX)) {
            aFull = aFull.substr(MERGE_OF_PREFIX.length());
        }
        if (boost::starts_with(bFull, MERGE_OF_PREFIX)) {
            bFull = bFull.substr(MERGE_OF_PREFIX.length());
        }

        mergedDesc = MERGE_OF_PREFIX + aFull + "," + bFull;

        mergedStartLine = -1;
        mergedEndLine = -1;
    }

    if (a->commentsOrNull.size() == b->commentsOrNull.size() && std::equal(a->commentsOrNull.begin(), a->commentsOrNull.end(), b->commentsOrNull.begin())) {
        mergedComments = a->commentsOrNull;
    }
    else {
        mergedComments.insert(mergedComments.end(), a->commentsOrNull.begin(), a->commentsOrNull.end());
        mergedComments.insert(mergedComments.end(), b->commentsOrNull.begin(), b->commentsOrNull.end());
    }

    return SimpleConfigOrigin::make_instance(mergedDesc, mergedStartLine, mergedEndLine, mergedType, mergedComments);
}

uint32_t SimpleConfigOrigin::similarity(const SimpleConfigOriginPtr& a, const SimpleConfigOriginPtr& b) {
    uint32_t count = 0;

    if (a->originType == b->originType) {
        count += 1;
    }

    if (a->description_ == b->description_) {
        count += 1;

        // only count these if the description field (which is the file
        // or resource name) also matches.
        if (a->lineNumber_ == b->lineNumber_) {
            count += 1;
        }
        if (a->endLineNumber == b->endLineNumber) {
            count += 1;
        }
    }

    return count;
}

SimpleConfigOriginPtr SimpleConfigOrigin::mergeThree(const SimpleConfigOriginPtr& a, const SimpleConfigOriginPtr& b, const SimpleConfigOriginPtr& c) {
    if (similarity(a, b) >= similarity(b, c)) {
        return mergeTwo(mergeTwo(a, b), c);
    }
    else {
        return mergeTwo(a, mergeTwo(b, c));
    }
}

ConfigOriginPtr SimpleConfigOrigin::mergeOrigins(const ConfigOriginPtr& a, const ConfigOriginPtr& b) {
    return mergeTwo(std::dynamic_pointer_cast<SimpleConfigOrigin>(a), std::dynamic_pointer_cast<SimpleConfigOrigin>(b));
}

ConfigOriginPtr SimpleConfigOrigin::mergeOrigins(const VectorAbstractConfigValue& stack) {
    VectorConfigOrigin origins;
    origins.reserve(stack.size());
    for (auto& v : stack) {
        origins.push_back(v->origin());
    }
    return mergeOrigins(origins);
}

ConfigOriginPtr SimpleConfigOrigin::mergeOrigins(const VectorConfigOrigin& stack) {
    if (stack.empty()) {
        throw ConfigExceptionBugOrBroken("can't merge empty list of origins");
    }
    else if (stack.size() == 1) {
        return stack.front();
    }
    else if (stack.size() == 2) {
        return mergeTwo(std::dynamic_pointer_cast<SimpleConfigOrigin>(stack[0]), std::dynamic_pointer_cast<SimpleConfigOrigin>(stack[1]));
    }
    else {
        VectorConfigOrigin remaining(stack);
        while (remaining.size() > 2) {
            auto c = std::dynamic_pointer_cast<SimpleConfigOrigin>(remaining.back());
            remaining.pop_back();
            auto b = std::dynamic_pointer_cast<SimpleConfigOrigin>(remaining.back());
            remaining.pop_back();
            auto a = std::dynamic_pointer_cast<SimpleConfigOrigin>(remaining.back());
            remaining.pop_back();

            auto merged = mergeThree(a, b, c);
            remaining.push_back(merged);
        }

        // should be down to either 1 or 2
        return mergeOrigins(remaining);
    }
}

}
