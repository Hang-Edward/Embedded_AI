#include "ShellAudioRecorder.h"

#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <utility>

namespace {

std::string quoteForShell(const std::string& value) {
    std::string quoted = "'";
    for (const char ch : value) {
        if (ch == '\'') {
            quoted += "'\\''";
        } else {
            quoted.push_back(ch);
        }
    }
    quoted += "'";
    return quoted;
}

} // namespace

ShellAudioRecorder::ShellAudioRecorder(std::string audioDevice)
    : audioDevice_(std::move(audioDevice)) {
}

AudioRecordResult ShellAudioRecorder::recordWav(const std::string& outputPath, int seconds) {
    AudioRecordResult result;
    result.filePath = outputPath;

    const auto parentPath = std::filesystem::path(outputPath).parent_path();
    if (!parentPath.empty()) {
        std::filesystem::create_directories(parentPath);
    }

#if defined(_WIN32)
    result.message = "Audio recording is only implemented on Raspberry Pi/Linux with arecord.";
    return result;
#else
    std::ostringstream command;
    command << "arecord -q -D " << quoteForShell(audioDevice_)
            << " -f S16_LE -r 16000 -c 1 -d "
            << seconds << " " << quoteForShell(outputPath);

    const int code = std::system(command.str().c_str());
    if (code != 0) {
        result.message = "arecord failed on device " + audioDevice_ + ". Check Logitech C270 microphone with: arecord -l";
        return result;
    }

    result.success = true;
    result.message = "Audio recorded.";
    return result;
#endif
}
