#include <map>
#include <utility>
#include <string>
#include <SDL2/SDL.h>

#include "oss_texture.h"
#include "oss_statesprite.h"

OSS_StateSprite::OSS_StateSprite()
{
    currentState = "";
    currentSubState = 0;
    substateRect = {0, 0, 0, 0};
    stateTexture = NULL;
    horizontalFlag = true;
    totalCurrentSegments = 1;
}

OSS_StateSprite::~OSS_StateSprite()
{
    states.clear();
}

bool OSS_StateSprite::addState(string state, OSS_Texture* texture, Uint16 clipData)
{
    if (texture != NULL)
    {
        states[state] = {texture, clipData};
        if (stateTexture == NULL)
        {
            changeState(state);
        }
        return true;
    }
    return false;
}

bool OSS_StateSprite::addState(string state, OSS_Texture* texture, bool horizontal, Uint16 segments)
{
    if (segments > OSS_STATE_HORIZONTAL)
    {
        segments = OSS_STATE_HORIZONTAL;
    }
    /// Convert segments into clipData - final bit indicating which axis to clip along
    return addState(state, texture, (Uint16)(segments | ((Uint16)horizontal * OSS_STATE_HORIZONTAL)));
}

bool OSS_StateSprite::changeState(string& state)
{
    map<string, pair<OSS_Texture*, Uint16>>::iterator i = states.find(state);
    if (i != states.end())
    {
        currentState = state;
        currentSubState = 0;
        stateTexture = i->second.first;
        totalCurrentSegments = i->second.second;
        horizontalFlag = ((totalCurrentSegments & OSS_STATE_HORIZONTAL) == OSS_STATE_HORIZONTAL);
        if (horizontalFlag)
        {
            /// Lop off the last bit to get the value we care about
            totalCurrentSegments = totalCurrentSegments ^ OSS_STATE_HORIZONTAL;
        }
        /// Update the substate to first substate
        changeSubState(0, true);
        return true;
    }
    return false;
}

void OSS_StateSprite::changeSubState(Uint16 substate, bool forceChange)
{
    if (substate >= totalCurrentSegments)
    {
        substate = 0;
    }
    if (!forceChange && substate == currentSubState)
    {
        /// No need to do more calculations if we're already in the desired substate
        return;
    }
    if (totalCurrentSegments == 0)
    {
        totalCurrentSegments = 1;
    }
    currentSubState = substate;
    if (!horizontalFlag)
    {
        int clipWidth = (stateTexture->getWidth() / (int)totalCurrentSegments);
        if (clipWidth < 1)
        {
            clipWidth = 1;
        }
        substateRect = {clipWidth * substate, 0, clipWidth, stateTexture->getHeight()};
    }
    else
    {
        int clipHeight = (stateTexture->getHeight() / (int)totalCurrentSegments);
        if (clipHeight < 1)
        {
            clipHeight = 1;
        }
        substateRect = {0, clipHeight * substate, stateTexture->getWidth(), clipHeight};
    }
}
/*
void OSS_StateSprite::splitState(string& state, Uint16 segments, bool horizontal)
{
    map<string, pair<OSS_Texture*, Uint16>>::iterator i = states.find(state);
    if (i != states.end())
    {
        if (segments == 0)
        {
            segments = 1;
        }
        segments = segments | ((Uint16)horizontal * OSS_STATE_HORIZONTAL);
        i->second.second = segments;
        if (state == currentState)
        {
            /// Change state will update the current clip rect accordingly
            changeState(state);
        }
    }
}
*/
string OSS_StateSprite::getCurrentState()
{
    return currentState;
}

Uint16 OSS_StateSprite::getCurrentSubstate()
{
    return currentSubState;
}

void OSS_StateSprite::render(SDL_Renderer* renderer, int x, int y, float angle, SDL_Point* origin, SDL_RendererFlip flip)
{
    stateTexture->render(renderer, x, y, &substateRect, angle, origin, flip);
}

void OSS_StateSprite::render(SDL_Renderer* renderer, SDL_Rect dest, float angle, SDL_Point* origin, SDL_RendererFlip flip)
{
    stateTexture->render(renderer, dest, &substateRect, angle, origin, flip);
}

void OSS_StateSprite::renderSimple(SDL_Renderer* renderer, int x, int y)
{
    stateTexture->renderSimple(renderer, x, y, &substateRect);
}