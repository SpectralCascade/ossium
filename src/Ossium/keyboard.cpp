#include "keyboard.h"

namespace Ossium
{

    inline namespace input
    {

        REGISTER_INPUT_HANDLER(KeyboardHandler);

        ActionOutcome KeyboardHandler::HandleInput(const SDL_Event& raw)
        {
            if (raw.type == SDL_KEYUP || raw.type == SDL_KEYDOWN)
            {
                /// Extract useful data
                KeyboardInput data;
                data.state = raw.type == SDL_KEYDOWN ? KEY_DOWN : KEY_UP;
                data.key = raw.key.keysym.sym;
                /// Set relevant state
                auto itr = _state_map.find(data.key);
                if (itr != _state_map.end())
                {
                    itr->second = data.state;
                }
                /// Find a bound action linked to this input condition
                return CallAction(data, data.key);
            }
            return ActionOutcome::Ignore;
        }

    }

}
