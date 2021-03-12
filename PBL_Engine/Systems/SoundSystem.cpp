// ///////////////////////////////////////////////////////////////// Includes //
#include "SoundSystem.hpp"

#include <soloud.h>
#include <soloud_wav.h>

#include <filesystem>
#include <random>

#include "Components/Components.hpp"
#include "ECS/ECS.hpp"

// //////////////////////////////////////////////////////// Namespace aliases //
namespace fs = std::filesystem;

// /////////////////////////////////////////////////////////////////// Usings //
using Path = std::string;
using FileExtension = std::string;

// ////////////////////////////////////////////////////////////////// Structs //
struct MultisampleEffect {
    Path path;
    FileExtension extension;
    std::vector<Path> samplePaths;
    int nextSample;
};

// ///////////////////////////////////////////////////////// Global variables //
SoLoud::Soloud audioEngine;
std::unordered_map<std::string, SoLoud::Wav> audioSources;
std::unordered_map<FileExtension, std::vector<Path>> audioPaths;
std::unordered_map<std::string, MultisampleEffect> multisampleEffects;

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

void SoundSystem::playRandomSample(std::string const &name,
                                   float const volume) {
    auto &effect = multisampleEffects.at(name);

    play(effect.samplePaths.at(effect.nextSample), volume);

    // Choose a next sample (not the same as the previous one, though)
    static std::random_device rng;
    static std::default_random_engine engine{rng()};
    std::uniform_int_distribution<int> distribution(
        0, effect.samplePaths.size() - 1);

    auto const previousSample = effect.nextSample;
    while ((effect.nextSample = distribution(engine)) == previousSample) {
    }
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

// --------------------------------------------------- Multiple samples -- == //
std::vector<Path> findSamplesInDirectory(Path const &path,
                                         FileExtension const &extension) {
    std::vector<Path> samples;
    for (auto const &entry : fs::recursive_directory_iterator(path)) {
        Path const &entryPath = entry.path().string();
        FileExtension const &entryExtension = entry.path().extension().string();

        if (entryExtension == extension) {
            samples.push_back(entryPath);
        }
    }
    return samples;
}

void SoundSystem::registerMultisampleEffect(std::string const &name,
                                            Path const &path,
                                            FileExtension const &extension) {
    multisampleEffects[name] = {
        .path = path,
        .extension = extension,
        .samplePaths = findSamplesInDirectory(path, extension),
        .nextSample = 0};
}

// ////////////////////////////////////////////////////////////////////////// //
