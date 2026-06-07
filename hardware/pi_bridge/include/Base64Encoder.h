#pragma once

#include <cstdint>
#include <string>
#include <vector>

class Base64Encoder {
public:
    std::string encode(const std::vector<uint8_t>& bytes) const;
};
