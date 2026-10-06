#include "Platform/Media.h"
#include <blkid/blkid.h>
#include <mntent.h>
#include <cstdlib>
#include <cstring>
#include <memory>

namespace {
struct Cache {
    blkid_cache Value = nullptr;
    ~Cache() { if (Value) blkid_put_cache(Value); }
};
}

std::vector<Platform::MediaVolume> Platform::OpticalMedia()
{
    std::vector<MediaVolume> volumes;
    Cache cache;
    if (blkid_get_cache(&cache.Value, nullptr) != 0) return volumes;
    std::unique_ptr<FILE, decltype(&endmntent)> mounts(setmntent("/proc/self/mounts", "r"), endmntent);
    if (!mounts) return volumes;
    mntent entry{};
    char buffer[8192];
    while (getmntent_r(mounts.get(), &entry, buffer, sizeof(buffer))) {
        if (std::strcmp(entry.mnt_type, "iso9660") != 0 && std::strcmp(entry.mnt_type, "udf") != 0) continue;
        std::unique_ptr<char, decltype(&std::free)> label(
            blkid_get_tag_value(cache.Value, "LABEL", entry.mnt_fsname), std::free);
        if (label) volumes.push_back({entry.mnt_dir, label.get()});
    }
    return volumes;
}
