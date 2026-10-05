#include "Platform/UI/Input.h"
#include "Platform/Windows/TextTypes.h"
float Platform::MouseWheelScrollLines() {
    UINT lines = 3;
    SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &lines, 0);
    return static_cast<float>(lines);
}
