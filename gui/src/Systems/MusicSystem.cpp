#include "Systems/MusicSystem.hpp"
#include "Components/ComponentInhabitant.hpp"
#include "Components/ComponentMusic.hpp"
#include "Components/ComponentShared.hpp"
#include "Logging/Logger.hpp"
#include <cstdlib>

namespace zappy {

void MusicSystem::update(World& w, float dt) {
    auto storage = w.get_storage<ComponentMusic>();
    if (!storage) {
        return;
    }

    std::vector<Entity> toRemove;

    for (auto& [entity, songPtr] : *storage) {
        auto& song = *songPtr;

        // Start a new song
        if (!song.isStarted && !song.isPlaying) {
            song.song.music = LoadMusicStream(song.song.sourcePath.c_str());
            PlayMusicStream(song.song.music);
            song.isStarted = true;
            song.isPlaying = true;
        }

        // Set the general music volume from the settings
        if (song.isPlaying) {
            SetMusicVolume(song.song.music, _volume);
        }

        // Play the song
        if (song.isPlaying) {
            UpdateMusicStream(song.song.music);
        }

        // Check if the song is at his end, if it is stop the stream and set isFinished as true
        if (song.isPlaying &&
            GetMusicTimePlayed(song.song.music) >= GetMusicTimeLength(song.song.music)) {
            StopMusicStream(song.song.music);
            song.isFinished = true;
            song.isPlaying = false;
        }

        // If the song is finished but his in loop mod, start it again
        if (song.loop && song.isFinished) {
            PlayMusicStream(song.song.music);
            song.isFinished = false;
        }

        // If the song is finished, add him to the list of component to remove
        if (!song.loop && song.isFinished) {
            toRemove.push_back(entity);
        }
    }

    for (const auto& entity : toRemove) {
        w.despawn(entity);
    }
}

float MusicSystem::getVolume() { return _volume; }

void MusicSystem::setVolume(float volume) { _volume = volume; }

void MusicSystem::volumeUp() {
    _volume += 0.1;
    if (_volume > 1.0) {
        _volume = 1.0;
    }
}

void MusicSystem::volumeDown() {
    _volume -= 0.1;
    if (_volume < 0.0) {
        _volume = 0.0;
    }
}

} // namespace zappy
