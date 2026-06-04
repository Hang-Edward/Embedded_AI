#include "HttpClient.h"

bool HttpResponse::ok() const {
    return statusCode >= 200 && statusCode < 300 && errorMessage.empty();
}
