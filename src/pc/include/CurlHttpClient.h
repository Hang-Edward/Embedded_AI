#pragma once

#include "HttpClient.h"

class CurlHttpClient final : public HttpClient {
public:
    HttpResponse postJson(const std::string& url,
        const std::map<std::string, std::string>& headers,
        const std::string& body) override;

private:
    static size_t writeBody(char* ptr, size_t size, size_t nmemb, void* userdata);
};
