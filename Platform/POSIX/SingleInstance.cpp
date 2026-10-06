#include "Platform/Application.h"
#include "Platform/Paths.h"
#include <SDL3/SDL_error.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

namespace {
struct Instance {
    int descriptor;
};
}

void* Platform::AcquireApplicationInstance(const char* identifier, bool allowMultiple)
{
    const std::string path = UserPath((std::string(identifier) + ".lock").c_str());
    const int descriptor = open(path.c_str(), O_CREAT | O_RDWR | O_CLOEXEC, 0600);
    if (descriptor < 0) {
        SDL_SetError("Could not open application lock: %s", std::strerror(errno));
        return nullptr;
    }
    if (flock(descriptor, LOCK_EX | LOCK_NB) != 0) {
        const int error = errno;
        close(descriptor);
        if ((error == EWOULDBLOCK || error == EAGAIN) && allowMultiple) return new Instance{-1};
        SDL_SetError("Could not acquire application lock: %s", std::strerror(error));
        return nullptr;
    }
    return new Instance{descriptor};
}

void Platform::ReleaseApplicationInstance(void* instance)
{
    auto* lock = static_cast<Instance*>(instance);
    if (!lock) return;
    if (lock->descriptor >= 0) close(lock->descriptor);
    delete lock;
}
