#pragma once

#include "CameraService.h"

class OpenCvCameraService : public CameraService {
public:
    explicit OpenCvCameraService(int preferredCameraIndex);

    CaptureResult captureFrame(const std::string& outputPath) override;

private:
    int findWorkingCameraIndex() const;
    bool tryReadFrame(int cameraIndex, const std::string& outputPath, CaptureResult& result) const;

    int preferredCameraIndex_;
};
