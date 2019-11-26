#pragma once

#ifndef DELTA_H
#define DELTA_H

#include "jsondata.h"

namespace Ossium
{

    class OSSIUM_EDL Delta
    {
    public:
        Delta();

        /// Get the change in time between calls to Update()
        float Time();

        /// Updates values in the class each time it is called
        void Update();

        /// Applies FPS capping configuration and initialises previousTicks to current ticks
        void Init(JSON& config);
        void Init();

        /// Resets previousTicks to current ticks
        void Reset();

    private:
        Uint32 previousTicks;

        float deltaTime;

        float fpscap;

    };

}

#endif // DELTA_H
