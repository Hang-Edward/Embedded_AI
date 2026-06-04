#include "Base64Encoder.h"

std::string Base64Encoder::encode(const std::vector<uint8_t>& bytes) const {
    static constexpr char kAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string output;
    output.reserve(((bytes.size() + 2U) / 3U) * 4U);

    for (size_t i = 0; i < bytes.size(); i += 3U) {
        const uint32_t a = bytes[i];
        const uint32_t b = (i + 1U < bytes.size()) ? bytes[i + 1U] : 0U;
        const uint32_t c = (i + 2U < bytes.size()) ? bytes[i + 2U] : 0U;
        const uint32_t triple = (a << 16U) | (b << 8U) | c;

        output.push_back(kAlphabet[(triple >> 18U) & 0x3FU]);
        output.push_back(kAlphabet[(triple >> 12U) & 0x3FU]);
        output.push_back(i + 1U < bytes.size() ? kAlphabet[(triple >> 6U) & 0x3FU] : '=');
        output.push_back(i + 2U < bytes.size() ? kAlphabet[triple & 0x3FU] : '=');
    }

    return output;
}
