// ///////////////////////////////////////////////////////////////// Includes //
#include "SoundSystem.hpp"

#include <soloud.h>
#include <soloud_wav.h>

#include <filesystem>

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"

// //////////////////////////////////////////////////////// Namespace aliases //
namespace fs = std::filesystem;

// /////////////////////////////////////////////////////////////////// Usings //
using Path = std::string;
using FileExtension = std::string;

// ///////////////////////////////////////////////////////// Global variables //
SoLoud::Soloud audioEngine;
std::unordered_map<std::string, SoLoud::Wav> audioSources;
std::unordered_map<FileExtension, std::vector<Path>> audioPaths;

// /////////////////////////////////////////////////////////////////// System //
// ============================================================= Behaviour == //
// ----------------------------------------- System's virtual functions -- == //
void SoundSystem::filters() {}

void SoundSystem::setup() {
    // Initialize the audio engine
    audioEngine.init(
        SoLoud::Soloud::CLIP_ROUNDOFF |   // Leave the defaults
        SoLoud::Soloud::LEFT_HANDED_3D);  // Add left-handedness for DirectX

    // Get all audio files in the assets directory
    Path const searchedDirectory{"Assets\\Audio"};
    std::vector<FileExtension> const searchedFileExtensions{".wav"};

    for (auto const &entry :
         fs::recursive_directory_iterator(searchedDirectory)) {
        Path const &path = entry.path().string();
        FileExtension const &extension = entry.path().extension().string();

        if (std::find(searchedFileExtensions.begin(),
                      searchedFileExtensions.end(),
                      extension) != searchedFileExtensions.end()) {
            audioPaths[extension].emplace_back(path);
        }
    }

    // Load the audio into memory
    for (auto const &[fileExtension, paths] : audioPaths) {
        for (auto const &path : paths) {
            audioSources.insert({path, SoLoud::Wav{}});
            audioSources.at(path).load(path.c_str());
        }
    }
}

void SoundSystem::update(float const deltaTime) {
    // Update 3D audio
    audioEngine.update3dAudio();
};

void SoundSystem::release() {
    // Release the audio engine
    audioEngine.deinit();
}

// ---------------------------------------------------- SoLoud wrappers -- == //
void SoundSystem::play(std::string const &sound, float const volume) {
    audioEngine.play(audioSources.at(sound), volume);
}

void SoundSystem::play3d(std::string const &sound, DirectX::XMFLOAT3 position,
                         float const volume) {
    audioSources.at(sound).set3dAttenuation(
        SoLoud::AudioSource::INVERSE_DISTANCE, 0.1f);
    audioEngine.play3d(audioSources.at(sound), position.x, position.y,
                       position.z, 0.0f, 0.0f, 0.0f, volume);
}

void SoundSystem::setListener(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 at,
                              DirectX::XMFLOAT3 up) {
    audioEngine.set3dListenerParameters(position.x, position.y, position.z,
                                        at.x, at.y, at.z, up.x, up.y, up.z, 0,
                                        0, 0);
}

// ////////////////////////////////////////////////////////////////////////// //
