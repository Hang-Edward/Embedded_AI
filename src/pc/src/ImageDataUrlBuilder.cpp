#include "ImageDataUrlBuilder.h"

#include "Base64Encoder.h"

#include <cstdint>
#include <fstream>
#include <iterator>
#include <vector>

std::string ImageDataUrlBuilder::buildJpegDataUrl(const std::string& imagePath) const {
    std::ifstream file(imagePath, std::ios::binary);
    if (!file) {
        return {};
    }

    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (bytes.empty()) {
        return {};
    }

    const Base64Encoder encoder;
    return "data:image/jpeg;base64," + encoder.encode(bytes);
}
