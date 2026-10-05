#pragma once
#include <cstdint>
#ifdef _WIN32
#include "Platform/Windows/Processor.h"
#else
#include "Platform/POSIX/Processor.h"
#endif
