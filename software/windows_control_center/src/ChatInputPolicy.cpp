#include "ChatInputPolicy.h"

bool ChatInputPolicy::shouldSubmit(int key,
                                   Qt::KeyboardModifiers modifiers,
                                   bool autoRepeat) {
    const bool enterPressed = key == Qt::Key_Return || key == Qt::Key_Enter;
    const bool shiftPressed = modifiers.testFlag(Qt::ShiftModifier);
    return enterPressed && !shiftPressed && !autoRepeat;
}
