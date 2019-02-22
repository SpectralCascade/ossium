#ifndef STATESPRITE_H
#define STATESPRITE_H

#include <map>
#include <utility>
#include <string>
#include <SDL.h>

#include "texture.h"
#include "resourcecontroller.h"
#include "renderer.h"

using namespace std;
/*
namespace Ossium
{
    /// Bit masks for flag that decides whether a state texture is split along x or y axis
    /// For use with Uint16 variables, so 15 bits are freely available
    #define STATE_HORIZONTAL        32768
    #define STATE_VERTICAL              0

    /// Can be switched between different textures/texture clips
    class StateSprite
    {
    public:
        StateSprite();
        ~StateSprite();

        /// Sets the final bit to whatever the boolean flag is, rather than worry about bit masks
        bool addState(string state, Texture* texture, bool horizontal = true, Uint16 segments = 1);

        /// Changes state and initialises current substate to 0
        bool changeState(string& state);

        /// Changes current substate clip segment
        void changeSubState(Uint16 substate, bool forceChange = false);

        /// Returns the current state
        string getCurrentState();

        /// Returns a pointer to the current state texture
        Texture* getCurrentTexture();

        /// Returns the current substate segment number
        Uint16 getCurrentSubstate();

        /// Sends the current state to the provided renderer
        void render(Renderer& renderer, int x, int y, int layer = 0, float angle = 0.0, SDL_Point* origin = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);

        /// Overload for simplicity
        void render(Renderer& renderer, SDL_Rect dest, int layer = 0, float angle = 0.0, SDL_Point* origin = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);

        /// Very simple alternative render methods
        void renderSimple(Renderer& renderer, int x, int y, int layer = 0);
        void renderSimple(Renderer& renderer, SDL_Rect dest, int layer = 0);

    protected:
        /// Prohibited copying for safety
        StateSprite(const StateSprite& thisCopy);
        StateSprite operator=(const StateSprite& thisCopy);

        /// Original addState method - this one does worry about bit masks, hence why it's protected
        bool addState(string state, Texture* texture, Uint16 clipData = 1 | STATE_HORIZONTAL);

        /// Multiple states, multiple textures
        /// Key = state, pair first = pointer to texture resource, pair second = clipping info
        /// First 15 bits of clipping info = number of segments
        /// Final bit decides whether clipping along horizontal or vertical
        map<string, pair<Texture*, Uint16>> states;

        /// Current state key
        string currentState;

        /// Current state texture
        Texture* stateTexture;

        /// Current substate
        Uint16 currentSubState;

        /// Current horizontal flag
        bool horizontalFlag;

        /// Current number of segments
        Uint16 totalCurrentSegments;

        /// Current substate clip rect to use
        SDL_Rect substateRect;

    };

}
*/
#endif // STATESPRITE_H