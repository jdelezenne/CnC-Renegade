#include "Platform/Threads.h"
#include <atomic>

std::uint32_t Platform::CurrentThreadId()
{
    static std::atomic<std::uint32_t> next{1};
    thread_local const auto identifier = next.fetch_add(1, std::memory_order_relaxed);
    return identifier;
}
