#include "CurlHttpClient.h"

#include <curl/curl.h>

#include <memory>
#include <sstream>

namespace {

struct CurlSlistDeleter {
    void operator()(curl_slist* list) const {
        if (list != nullptr) {
            curl_slist_free_all(list);
        }
    }
};

std::unique_ptr<curl_slist, CurlSlistDeleter> buildHeaders(const std::map<std::string, std::string>& headers) {
    curl_slist* rawHeaders = nullptr;
    rawHeaders = curl_slist_append(rawHeaders, "Content-Type: application/json");

    for (const auto& [name, value] : headers) {
        rawHeaders = curl_slist_append(rawHeaders, (name + ": " + value).c_str());
    }

    return std::unique_ptr<curl_slist, CurlSlistDeleter>(rawHeaders);
}

} // namespace

HttpResponse CurlHttpClient::postJson(const std::string& url,
    const std::map<std::string, std::string>& headers,
    const std::string& body) {
    HttpResponse response;

    CURL* curl = curl_easy_init();
    if (curl == nullptr) {
        response.errorMessage = "curl_easy_init failed.";
        return response;
    }

    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curlGuard(curl, curl_easy_cleanup);
    auto headerList = buildHeaders(headers);

    std::string responseBody;
    char errorBuffer[CURL_ERROR_SIZE] {};

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList.get());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &CurlHttpClient::writeBody);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 90L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    const CURLcode code = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.statusCode);
    response.body = responseBody;

    if (code != CURLE_OK) {
        response.errorMessage = errorBuffer[0] != '\0' ? errorBuffer : curl_easy_strerror(code);
    }

    return response;
}

size_t CurlHttpClient::writeBody(char* ptr, size_t size, size_t nmemb, void* userdata) {
    const size_t total = size * nmemb;
    auto* response = static_cast<std::string*>(userdata);
    response->append(ptr, total);
    return total;
}
