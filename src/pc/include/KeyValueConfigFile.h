#pragma once

#include <map>
#include <string>

class KeyValueConfigFile {
public:
    bool load(const std::string& filePath);
    std::string getString(const std::string& key, const std::string& fallback) const;
    bool has(const std::string& key) const;

private:
    static std::string trim(const std::string& value);
    static bool isCommentOrBlank(const std::string& line);

    std::map<std::string, std::string> values_;
};
