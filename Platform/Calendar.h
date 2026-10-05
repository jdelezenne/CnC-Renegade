#pragma once
#ifdef _WIN32
#include "Platform/Windows/TextTypes.h"
namespace Platform { using CalendarTime = SYSTEMTIME; }
#else
#include <cstdint>
namespace Platform {
struct CalendarTime {
    std::uint16_t wYear, wMonth, wDayOfWeek, wDay, wHour, wMinute, wSecond, wMilliseconds;
};
}
#endif
namespace Platform {
void LocalCalendarTime(CalendarTime& time);
void UtcCalendarTime(CalendarTime& time);
}
