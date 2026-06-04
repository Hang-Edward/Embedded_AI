#include "QwenResponseParser.h"

namespace {

std::string jsonKey(const std::string& key) {
    return "\"" + key + "\"";
}

} // namespace

std::string QwenResponseParser::extractAssistantText(const std::string& responseJson) const {
    // 中文注释：百炼 OpenAI 兼容响应通常从 choices[0].message.content 读取文本。
    const std::string content = extractStringAfterKey(responseJson, "content");
    if (!content.empty()) {
        return content;
    }
    return extractStringAfterKey(responseJson, "reasoning_content");
}

std::string QwenResponseParser::extractErrorMessage(const std::string& responseJson) const {
    const std::string message = extractStringAfterKey(responseJson, "message");
    if (!message.empty()) {
        return message;
    }
    return extractStringAfterKey(responseJson, "code");
}

std::string QwenResponseParser::extractStringAfterKey(const std::string& json, const std::string& key) {
    const std::string marker = jsonKey(key);
    const auto keyPos = json.find(marker);
    if (keyPos == std::string::npos) {
        return {};
    }

    const auto colon = json.find(':', keyPos + marker.size());
    if (colon == std::string::npos) {
        return {};
    }

    const auto quote = json.find('"', colon + 1U);
    if (quote == std::string::npos) {
        return {};
    }

    std::string raw;
    bool escaped = false;
    for (size_t i = quote + 1U; i < json.size(); ++i) {
        const char ch = json[i];
        if (escaped) {
            raw.push_back('\\');
            raw.push_back(ch);
            escaped = false;
            continue;
        }
        if (ch == '\\') {
            escaped = true;
            continue;
        }
        if (ch == '"') {
            return unescapeJsonString(raw);
        }
        raw.push_back(ch);
    }
    return {};
}

std::string QwenResponseParser::unescapeJsonString(const std::string& value) {
    std::string result;
    result.reserve(value.size());

    for (size_t i = 0; i < value.size(); ++i) {
        if (value[i] != '\\' || i + 1U >= value.size()) {
            result.push_back(value[i]);
            continue;
        }

        const char escaped = value[++i];
        switch (escaped) {
        case 'n':
            result.push_back('\n');
            break;
        case 'r':
            result.push_back('\r');
            break;
        case 't':
            result.push_back('\t');
            break;
        case '"':
            result.push_back('"');
            break;
        case '\\':
            result.push_back('\\');
            break;
        default:
            result.push_back(escaped);
            break;
        }
    }

    return result;
}
