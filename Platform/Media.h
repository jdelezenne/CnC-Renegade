#pragma once

#include <string>
#include <vector>

namespace Platform {
struct MediaVolume {
    std::string Path;
    std::string Label;
};
std::vector<MediaVolume> OpticalMedia();
}
