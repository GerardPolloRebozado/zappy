#ifndef ZAPPY_COMPONENTMUSIC_HPP
#define ZAPPY_COMPONENTMUSIC_HPP

#include "Graphics/Music.hpp"
#include <string>
#include <vector>

namespace zappy {

struct ComponentMusic {
    Music song;
    float timePlayed;
    bool isPlaying = false;
    bool loop = false; // To play the song in loop forever
    bool isStarted = false;
    bool isFinished = false;
    float volume; // Volume is between 0.0f and 1.0f
    ComponentMusic(std::string source) { song.sourcePath = source; }
    ComponentMusic(std::string source, bool isLoop) : loop(isLoop) { song.sourcePath = source; }
};

} // namespace zappy

#endif // ZAPPY_COMPONENTMUSIC_HPP
