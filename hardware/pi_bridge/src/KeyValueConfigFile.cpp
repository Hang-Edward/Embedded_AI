#include "KeyValueConfigFile.h"

#include <algorithm>
#include <cctype>
#include <fstream>

bool KeyValueConfigFile::load(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (isCommentOrBlank(line)) {
            continue;
        }

        const auto equals = line.find('=');
        if (equals == std::string::npos) {
            continue;
        }

        const std::string key = trim(line.substr(0, equals));
        const std::string value = trim(line.substr(equals + 1U));
        if (!key.empty()) {
            values_[key] = value;
        }
    }

    return true;
}

std::string KeyValueConfigFile::getString(const std::string& key, const std::string& fallback) const {
    const auto it = values_.find(key);
    if (it == values_.end() || it->second.empty()) {
        return fallback;
    }
    return it->second;
}

bool KeyValueConfigFile::has(const std::string& key) const {
    return values_.find(key) != values_.end();
}

std::string KeyValueConfigFile::trim(const std::string& value) {
    auto begin = value.begin();
    while (begin != value.end() && std::isspace(static_cast<unsigned char>(*begin))) {
        ++begin;
    }

    auto end = value.end();
    while (end != begin && std::isspace(static_cast<unsigned char>(*(end - 1)))) {
        --end;
    }

    std::string result(begin, end);
    if (result.size() >= 2U && result.front() == '"' && result.back() == '"') {
        result = result.substr(1U, result.size() - 2U);
    }
    return result;
}

bool KeyValueConfigFile::isCommentOrBlank(const std::string& line) {
    const std::string stripped = trim(line);
    return stripped.empty() || stripped[0] == '#' || stripped[0] == ';';
}
