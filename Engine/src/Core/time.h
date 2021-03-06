/** COPYRIGHT NOTICE
 *  
 *  Ossium Engine
 *  Copyright (c) 2018-2020 Tim Lane
 *  
 *  This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
 *  
 *  Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *  
 *  1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
 *  
 *  2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
 *  
 *  3. This notice may not be removed or altered from any source distribution.
 *  
**/
#ifndef TIME_H
#define TIME_H

#include "schemamodel.h"
#include "callback.h"

namespace Ossium
{
    /// A special clock that measures relative time passed
    class OSSIUM_EDL Clock
    {
    public:
        /// Takes the 'absolute' time at which the clock began, in milliseconds.
        explicit Clock(Uint32 startTimeMS = 0);

        /// Updates the time passed on the clock according to the time scale factor. Returns true if wrapping occurred during the update.
        bool Update(float deltaTime);

        /// Pause/Resume time.
        void SetPaused(bool pause);

        /// Return pause state.
        bool IsPaused();

        /// Stretch clock time by some scale factor.
        void Scale(float scaleFactor);

        /// Returns the scale factor.
        float GetScaleFactor();

        /// Steps forward or backwards in time by a number of frames; defaults to 1/60th of a second per frame.
        void StepFrames(int frames = 1, float framePeriod = 1.0f / 60.0f);

        /// Returns relative time passed in milliseconds.
        Uint32 GetTime();

        /// Returns the 'absolute' time that the clock began in milliseconds.
        Uint32 GetInitialTime();

        /// Returns the relative time difference between Update() calls.
        float GetDeltaTime();

        /// Sets the time. DeltaTime is set to 0.
        void SetTime(Uint32 pos);

        /// Make this clock wraparound if it goes over this value. Set to 0 to disable wrapping.
        void SetWrap(Uint32 value);

        /// Returns the wraparound value.
        Uint32 GetWrap();

    private:
        /// Initial, 'absolute' time at which the clock began.
        Uint32 initialTime = 0;

        /// Relative time passed since clock began in milliseconds.
        Uint32 time = 0;

        /// Last value of time.
        Uint32 previousTime = 0;

        /// Scale factor to stretch/compress clock by.
        float scale = 1.0f;

        /// Whether or not the clock is paused.
        bool paused = false;

        /// The maximum value of time. Ignored if 0.
        Uint32 wrapValue = 0;

        /// Wrapping can cause issues with calculating delta time; this variable ensures no accuracy is lost if wrapping occurs.
        int overflow = 0;

    };

    /// A simple timer, like a stop watch; can only be used for positive timing.
    /// As such, use of this class is limited to timeframes that aren't reversable.
    class OSSIUM_EDL Timer
    {
    public:
        /// Takes a clock instance to provide timing ticks - otherwise defaults to absolute time.
        Timer(Clock* refClock = nullptr);
        Timer(const Timer& source);
        Timer& operator=(const Timer& source);

        /// Timer actions; all parameters are in milliseconds and generally correspond to time passed
        /// since some arbitrary clock began - if no clock object is used, utilise SDL_GetTicks().
        void Start();
        void Stop();
        void Pause();
        void Resume();

        /// Return the relative time since the timer has been started.
        Uint32 GetTicks();

        /// Return timer flags.
        bool IsStarted();
        bool IsPaused();

    protected:
        /// Reference clock.
        Clock* clock;

        /// Time at which the timer started, in milliseconds.
        Uint32 startTicks = 0;

        /// Ticks stored while timer is paused.
        Uint32 pausedTicks = 0;

        /// Timer flags.
        bool paused = false;
        bool started = false;

    };

    struct OSSIUM_EDL TimeSequenceSchema : public Schema<TimeSequenceSchema, 10>
    {
        DECLARE_BASE_SCHEMA(TimeSequenceSchema, 10);

        // Chronological array of time points, in milliseconds.
        M(std::vector<Uint32>, timePoints);

        // Values < 0 means repeat forever, 0 means never repeat, 1+ means repeat that many times.
        M(int, loops) = 0;

        // When true, after the final loop the timer is stopped and the event index is reset.
        // Otherwise, the timer is permanently paused and the event index is not reset.
        M(bool, resetOnFinish) = false;

    protected:
        // The current time point index.
        M(Uint32, eventIndex) = 0;
        
        // How many times the sequence has played.
        M(Uint32, currentLoop) = 0;

    };

    /// A time-based sequential event tracker; given a set of time points in milliseconds,
    /// returns the current corresponding sequence index. Useful for animating stuff or triggering events.
    /// The event index corresponds to the current time point. When an event time point is reached,
    /// the OnEventChange callback is called with the index of the event time point.
    class OSSIUM_EDL TimeSequence : public Timer, public TimeSequenceSchema
    {
    public:
        CONSTRUCT_SCHEMA(SchemaRoot, TimeSequenceSchema);

        TimeSequence(std::vector<Uint32> timePoints = {}, int loops = 0, Clock* refClock = nullptr);

        // Overload to reset eventIndex. Note, parent method is not virtual.
        void Start();

        // Overload to reset eventIndex. Note, parent method is not virtual.
        void Stop();

        // Checks the time and updates time tracking accordingly.
        void Update();

        // Returns the index to the next time point in the sequence.
        Uint32 GetEventIndex();

        // Returns the normalised difference of the current event index with the last event, between 0 and 1.
        float GetEventProgress();

        // Returns the normalised progress from the first time point to the last time point in the sequence
        float GetSequenceProgress();

        // Called when the eventIndex changes.
        Callback<Uint32> OnEventChange;

    };

    /// Converts seconds to milliseconds.
    OSSIUM_EDL Uint32 GetMS(float seconds);
    /// Converts milliseconds to seconds.
    OSSIUM_EDL float GetSeconds(Uint32 ms);

}

#endif // TIME_H
