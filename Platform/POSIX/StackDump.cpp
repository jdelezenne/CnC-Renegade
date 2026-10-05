#include "stackdump.h"
#include "wwdebug.h"
#include <execinfo.h>
#include <cstdlib>

void cStackDump::Print_Call_Stack()
{
    void* frames[256];
    const int count = backtrace(frames, 256);
    char** symbols = backtrace_symbols(frames, count);
    if (symbols) {
        for (int frame = 0; frame < count; ++frame) WWDEBUG_SAY(("%s\n", symbols[frame]));
        std::free(symbols);
    }
}
