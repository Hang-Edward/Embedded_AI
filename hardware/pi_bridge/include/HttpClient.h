#pragma once

#include <map>
#include <string>

struct HttpResponse {
    long statusCode = 0;
    std::string body;
    std::string errorMessage;

    bool ok() const;
};

class HttpClient {
public:
    virtual ~HttpClient() = default;

    virtual HttpResponse postJson(const std::string& url,
        const std::map<std::string, std::string>& headers,
        const std::string& body) = 0;
};
