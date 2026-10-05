#include "Platform/Calendar.h"
#include <chrono>
#include <ctime>
#include <system_error>
namespace {
void Calendar(Platform::CalendarTime& result, bool utc) {
    const auto now = std::chrono::system_clock::now();
    const auto seconds = std::chrono::floor<std::chrono::seconds>(now);
    const auto epoch = std::chrono::system_clock::to_time_t(seconds);
    std::tm time{};
    if (!(utc ? gmtime_r(&epoch, &time) : localtime_r(&epoch, &time)))
        throw std::system_error(errno, std::generic_category(), "Calendar time conversion");
    result = {static_cast<std::uint16_t>(time.tm_year + 1900), static_cast<std::uint16_t>(time.tm_mon + 1),
        static_cast<std::uint16_t>(time.tm_wday), static_cast<std::uint16_t>(time.tm_mday),
        static_cast<std::uint16_t>(time.tm_hour), static_cast<std::uint16_t>(time.tm_min),
        static_cast<std::uint16_t>(time.tm_sec),
        static_cast<std::uint16_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now - seconds).count())};
}
}
void Platform::LocalCalendarTime(CalendarTime& time) { Calendar(time, false); }
void Platform::UtcCalendarTime(CalendarTime& time) { Calendar(time, true); }
