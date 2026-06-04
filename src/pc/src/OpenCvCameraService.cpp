#include "OpenCvCameraService.h"

#include <filesystem>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>

OpenCvCameraService::OpenCvCameraService(int preferredCameraIndex)
    : preferredCameraIndex_(preferredCameraIndex) {
}

CaptureResult OpenCvCameraService::captureFrame(const std::string& outputPath) {
    CaptureResult result;
    const auto parentPath = std::filesystem::path(outputPath).parent_path();
    if (!parentPath.empty()) {
        std::filesystem::create_directories(parentPath);
    }

    const int cameraIndex = findWorkingCameraIndex();
    if (cameraIndex < 0) {
        result.message = "No readable camera found.";
        return result;
    }

    if (!tryReadFrame(cameraIndex, outputPath, result)) {
        return result;
    }

    result.success = true;
    result.cameraIndex = cameraIndex;
    result.filePath = outputPath;
    result.message = "Frame captured.";
    return result;
}

int OpenCvCameraService::findWorkingCameraIndex() const {
    if (preferredCameraIndex_ >= 0) {
        CaptureResult ignored;
        if (tryReadFrame(preferredCameraIndex_, "captures/.probe.jpg", ignored)) {
            std::filesystem::remove("captures/.probe.jpg");
            return preferredCameraIndex_;
        }
        return -1;
    }

    for (int index = 0; index < 6; ++index) {
        CaptureResult ignored;
        if (tryReadFrame(index, "captures/.probe.jpg", ignored)) {
            std::filesystem::remove("captures/.probe.jpg");
            return index;
        }
    }
    return -1;
}

bool OpenCvCameraService::tryReadFrame(int cameraIndex, const std::string& outputPath, CaptureResult& result) const {
    const auto parentPath = std::filesystem::path(outputPath).parent_path();
    if (!parentPath.empty()) {
        std::filesystem::create_directories(parentPath);
    }

    cv::VideoCapture camera(cameraIndex, cv::CAP_DSHOW);
    if (!camera.isOpened()) {
        result.message = "Camera index " + std::to_string(cameraIndex) + " cannot be opened.";
        return false;
    }

    camera.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    camera.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

    cv::Mat frame;
    for (int attempt = 0; attempt < 5; ++attempt) {
        camera.read(frame);
        if (!frame.empty()) {
            break;
        }
    }

    if (frame.empty()) {
        result.message = "Camera index " + std::to_string(cameraIndex) + " opened but returned empty frame.";
        return false;
    }

    result.width = frame.cols;
    result.height = frame.rows;
    result.cameraIndex = cameraIndex;
    if (!cv::imwrite(outputPath, frame)) {
        result.message = "Failed to save frame to " + outputPath;
        return false;
    }
    return true;
}
