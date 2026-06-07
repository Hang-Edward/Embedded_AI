#pragma once

#include <string>

struct AudioRecordResult {
    bool success = false;
    std::string filePath;
    std::string message;
};
