#include <cstdio>
#include <string>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#include "init.h"
#include "delta.h"
#include "ecs.h"
#include "audio.h"

using namespace std;

namespace Ossium
{
    int InitialiseOssium()
    {
        string numCPUs = "";
        SDL_Log("%s | %d core CPU | %d MB memory\n", SDL_GetPlatform(), SDL_GetCPUCount(), SDL_GetSystemRAM());

        /// Ensure errors are output to console if debug build (use "-D DEBUG" in GCC compile options)
        #ifdef DEBUG
        SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);
        #endif // DEBUG

        int error = 0;
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL error during initialisation: %s", SDL_GetError());
            error = INIT_ERROR_SDL;
        }
        else
        {
            int imgFlags = IMG_INIT_PNG;
            if (!(IMG_Init(imgFlags) & imgFlags))
            {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_Image error during initialisation: %s", IMG_GetError());
                error = INIT_ERROR_IMG;
            }
            else
            {
                if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
                {
                    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_Mixer error during initialisation: %s", Mix_GetError());
                    error = INIT_ERROR_MIXER;
                }
                else
                {
                    /// Initialise the audio channel subsystem
                    audio::internals::ChannelController::_Instance().Init(50);

                    #ifdef _SDL_TTF_H
                    if (TTF_Init() == -1)
                    {
                        SDL_LogError(SDL_LOG_CATEGORY_ERROR, TTF_GetError());
                        error = INIT_ERROR_TTF;
                    }
                    else
                    {
                        SDL_Log("Initialised OSSIUM ENGINE successfully!");
                    }
                    #endif // _SDL_TTF_H
                }
            }
        }
        return error;
    }

    void TerminateOssium()
    {
        Mix_CloseAudio();
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        printf("INFO: Successfully terminated Ossium.");
    }

}