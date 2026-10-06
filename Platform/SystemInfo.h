#pragma once
#include <string>
namespace Platform {
const char* OperatingSystemName(unsigned platform);
std::string ComputerName();
std::string UserName();
std::string ExecutablePath();
std::string ProcessStartTimeString();
}
