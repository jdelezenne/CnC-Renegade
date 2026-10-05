#include "Platform/Calendar.h"
void Platform::LocalCalendarTime(CalendarTime& time) { GetLocalTime(&time); }
void Platform::UtcCalendarTime(CalendarTime& time) { GetSystemTime(&time); }
