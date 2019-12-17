#ifndef AUDIOEXTENSIONS_H
#define AUDIOEXTENSIONS_H

#include "../Core/audio.h"
#include "../Core/component.h"

namespace Ossium
{

    inline namespace Audio
    {

        struct OSSIUM_EDL AudioSourceSchema : public Schema<AudioSourceSchema, 2>
        {
        public:
            DECLARE_BASE_SCHEMA(AudioSourceSchema, 2);

        protected:
            /// Path to the currently playing audio sample.
            M(string, samplePath);

            /// Is the source looping forever?
            M(bool, looping) = false;

        };

        namespace Internal
        {

            class OSSIUM_EDL AudioSourceSchemaCombiner : public AudioPlayer, public AudioSourceSchema
            {
            public:
                CONSTRUCT_SCHEMA(AudioPlayer, AudioSourceSchema);
            };

        }

        class OSSIUM_EDL AudioSource : public Component, public Internal::AudioSourceSchemaCombiner
        {
        public:
            CONSTRUCT_SCHEMA(Component, AudioSourceSchemaCombiner);
            DECLARE_COMPONENT(AudioSource);

            /// Plays an audio sample with panning and volume adjusted to simulate spatial audio for the main listener at a particular position.
            /// Doesn't account for walls and physics objects, purely distance and direction "as the crow flies" from the listener.
            void Play(AudioClip* sample, float vol = -1.0f, int repeats = 0);

            /// Updates the volume and panning of the audio playback for simple spatial effects.
            void Update();

            /// If a sample is specified, play it.
            void OnLoadFinish();

        private:
            /// Sets the spatial audio attenuation and panning of this source.
            void SetAudioPosition();

            using Internal::AudioSourceSchemaCombiner::Play;

        };

        struct OSSIUM_EDL AudioListenerSchema : public Schema<AudioListenerSchema, 1>
        {
            DECLARE_BASE_SCHEMA(AudioListenerSchema, 1);

            /// The minimum distance at which audio sources aren't listened to.
            M(float, cutoff) = 1280.0f;

        };

        class OSSIUM_EDL AudioListener : public Component, public AudioListenerSchema
        {
        public:
            DECLARE_COMPONENT(AudioListener);
            CONSTRUCT_SCHEMA(Component, AudioListenerSchema);

            friend class AudioSource;

            /// Sets the static mainListener instance to this instance if not already set.
            /// Logs a warning if the main listener is already set.
            void OnCreate();

            /// If this is the main listener, set the main listener pointer to nullptr.
            void OnDestroy();

        private:
            static AudioListener* mainListener;

        };

    }

}

#endif // AUDIOEXTENSIONS_H