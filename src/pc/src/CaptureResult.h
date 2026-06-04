#pragma once

#include <string>

struct CaptureResult {
    bool success = false;
    int cameraIndex = -1;
    int width = 0;
    int height = 0;
    std::string filePath;
    std::string message;
};
