#pragma once

#include <Qt>

class ChatInputPolicy {
public:
    static bool shouldSubmit(int key,
                             Qt::KeyboardModifiers modifiers,
                             bool autoRepeat = false);
};
