#include "srandom.h"
#include <sys/random.h>
#include <cerrno>
#include <system_error>

void SecureRandomClass::Generate_Seed()
{
    std::size_t offset = 0;
    while (offset < sizeof(Seeds)) {
        const auto count = getrandom(Seeds + offset, sizeof(Seeds) - offset, 0);
        if (count < 0) {
            if (errno == EINTR) continue;
            throw std::system_error(errno, std::generic_category(), "Unable to obtain random seed");
        }
        if (!count) throw std::system_error(EIO, std::generic_category(), "Empty random seed read");
        offset += static_cast<std::size_t>(count);
    }
}
