#pragma once

#include <string>

class QwenResponseParser {
public:
    std::string extractAssistantText(const std::string& responseJson) const;
    std::string extractErrorMessage(const std::string& responseJson) const;

private:
    static std::string extractStringAfterKey(const std::string& json, const std::string& key);
    static std::string unescapeJsonString(const std::string& value);
};
