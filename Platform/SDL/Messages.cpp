#include "msgloop.h"
#include "Platform/Platform.h"

void Windows_Message_Handler()
{
    Platform::PumpEvents();
}
