#ifndef ENGINESYSTEM_H
#define ENGINESYSTEM_H

#include "input.h"
#include "delta.h"
#include "component.h"

namespace Ossium
{

    class EngineSystem : public GlobalServices
    {
    public:
        friend class EntityComponentSystem;

        template<typename ...Args>
        EngineSystem(JSON& configData, Args&& ...services)
        {
            ecs = new EntityComponentSystem(forward<Args>(services)...);
            Init(configData);
        }

        template<typename ...Args>
        EngineSystem(string configFilePath = "", Args&& ...services)
        {
            ecs = new EntityComponentSystem(forward<Args>(services)...);
            if (!configFilePath.empty())
            {
                Init(configFilePath);
            }
        }

        ~EngineSystem();

        /// Configures the engine with a JSON file.
        void Init(string configFilePath);
        /// Configures the engine with a JSON object.
        void Init(JSON& configData);

        /// Executes the main game loop, including input handling and rendering.
        /// Returns false when the game should stop, otherwise returns true.
        bool Update();

        /// Loads a game scene into the entity component system.
        bool LoadScene(string path);

    private:
        NOCOPY(EngineSystem);

        /// The core entity-component system.
        EntityComponentSystem* ecs = nullptr;

        /// The main input controller.
        InputController input;

        /// SDL_Event for input handling.
        SDL_Event currentEvent;

        /// Time keeping for this system.
        Delta delta;

    };

}

#endif // ENGINESYSTEM_H