#pragma once

#include "AudioRecorder.h"

#include <string>

class ShellAudioRecorder final : public AudioRecorder {
public:
    explicit ShellAudioRecorder(std::string audioDevice);

    AudioRecordResult recordWav(const std::string& outputPath, int seconds) override;

private:
    std::string audioDevice_;
};
