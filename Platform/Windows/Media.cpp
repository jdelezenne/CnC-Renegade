#include "Platform/Media.h"
#include <windows.h>
#include <cstring>

std::vector<Platform::MediaVolume> Platform::OpticalMedia()
{
    std::vector<MediaVolume> volumes;
    const DWORD size = GetLogicalDriveStringsA(0, nullptr);
    if (!size) return volumes;
    std::vector<char> roots(size + 1);
    if (!GetLogicalDriveStringsA(static_cast<DWORD>(roots.size()), roots.data())) return volumes;
    for (const char* root = roots.data(); *root; root += std::strlen(root) + 1) {
        if (GetDriveTypeA(root) != DRIVE_CDROM) continue;
        char label[256]{};
        if (GetVolumeInformationA(root, label, sizeof(label), nullptr, nullptr, nullptr, nullptr, 0)) {
            volumes.push_back({root, label});
        }
    }
    return volumes;
}
