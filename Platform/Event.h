#pragma once

#include <atomic>
#include <memory>
#include <new>

namespace Platform {
class Event {
public:
    void Signal() { signaled.store(true, std::memory_order_release); }
    bool IsSignaled() const { return signaled.load(std::memory_order_acquire); }
private:
    std::atomic<bool> signaled{false};
};
using EventPointer = std::shared_ptr<Event>;
inline EventPointer MakeEvent()
{
    try { return std::make_shared<Event>(); }
    catch (const std::bad_alloc&) { return {}; }
}
}
