#ifndef ZAPPY_MUSIC_HPP
#define ZAPPY_MUSIC_HPP

#include <raylib-cpp.hpp>

namespace zappy {

struct Music {
    raylib::Music music; // Music oject from RayLib
    std::string sourcePath; // path of the music file (.mp3)
};

} // namespace zappy

#endif // ZAPPY_MUSIC_HPP
