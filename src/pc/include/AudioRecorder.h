#pragma once

#include "AudioRecordResult.h"

#include <string>

class AudioRecorder {
public:
    virtual ~AudioRecorder() = default;

    virtual AudioRecordResult recordWav(const std::string& outputPath, int seconds) = 0;
};
