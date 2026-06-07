#pragma once

#include "CaptureResult.h"

#include <string>

class CameraService {
public:
    virtual ~CameraService() = default;

    virtual CaptureResult captureFrame(const std::string& outputPath) = 0;
};
