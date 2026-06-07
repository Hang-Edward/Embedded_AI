#pragma once

#include <string>

class ImageDataUrlBuilder {
public:
    std::string buildJpegDataUrl(const std::string& imagePath) const;
};
