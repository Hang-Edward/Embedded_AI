#pragma once

#include "AudioRecorder.h"

class ShellAudioRecorder final : public AudioRecorder {
public:
    AudioRecordResult recordWav(const std::string& outputPath, int seconds) override;
};
