#include "OutputDevice.h"

bool BinaryOutputDevice::turnOff() {
    return setEnabled(false);
}
