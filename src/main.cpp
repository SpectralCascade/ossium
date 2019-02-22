#include <cstdio>
#include <string.h>
#include <SDL.h>

#include "Ossium/config.h"
#include "Ossium/init.h"
#include "Ossium/window.h"
#include "Ossium/time.h"
#include "Ossium/font.h"
#include "Ossium/text.h"
#include "Ossium/renderer.h"
#include "Ossium/ecs.h"
#include "Ossium/delta.h"
#include "Ossium/keyboard.h"
#include "Ossium/mouse.h"
#include "Ossium/audio.h"
#include "Ossium/colours.h"

#ifdef UNIT_TESTS
#include "Ossium/testmodules.h"
using namespace Ossium::test;
#endif // UNIT_TESTS

using namespace std;
using namespace Ossium;

Text* targetText = nullptr;
Text* mainText = nullptr;
int posx = 0;
int posy = 0;
bool check_for_key = false;
bool update_binding = false;
SDL_Keycode currentKey = SDLK_m;
float volume = 1.0f;
bool volume_change = false;
Sint16 panning = 0;
AudioBus master;

void MouseScrollAction(const MouseInput& data)
{
    posx -= data.x * 8;
    posy -= data.y * 8;
    volume += data.y * 0.02f;
    volume_change = true;
    panning -= data.y * 6;
}

void MouseClickAction(const MouseInput& data)
{
    if (mainText != nullptr)
    {
        if (data.state == MOUSE_RELEASED)
        {
            mainText->setColor(colours::GREEN);
            mainText->setText("Press any key to bind to action TOGGLE AUDIO");
            check_for_key = true;
        }
    }
}

void GetKey(const KeyboardInput& data)
{
    if (check_for_key && data.state == KEY_UP)
    {
        currentKey = data.key;
        mainText->setColor(colours::RED);
        mainText->setText("Current master mute key is " + (string)SDL_GetKeyName(currentKey));
        check_for_key = false;
        update_binding = true;
    }
}

void KeyAction(const KeyboardInput& data)
{
    if (targetText != nullptr)
    {
        if (data.state == KEY_DOWN)
        {
            targetText->setBackgroundColor(colours::YELLOW);
        }
        else if (data.state == KEY_UP)
        {
            if (!master.IsMuted())
            {
                targetText->setBackgroundColor(colours::RED);
                master.Mute();
            }
            else
            {
                master.Unmute();
                targetText->setBackgroundColor(colours::GREEN);
            }
        }
    }
}

int main(int argc, char* argv[])
{
    bool quit = false;
    if (InitialiseOssium() < 0)
    {
        printf("ERROR: Failed to initialise Ossium engine.\n");
    }
    else
    {
        #ifdef UNIT_TESTS
        TEST_RUN(CircularBufferTests);
        TEST_RUN(BasicUtilsTests);
        TEST_RUN(TreeTests);
        TEST_RUN(FSM_Tests);
        TEST_RUN(EventSystemTests);
        TEST_RUN(CSV_Tests);
        TEST_EVALUATE();
        return 0;
        #endif // UNIT_TESTS

        /// Load configuration settings
        Config settings;
        LoadConfig(&settings);

        /// Create the window
        Window mainWindow("Ossium Engine", 1024, 768, settings.fullscreen, SDL_WINDOW_SHOWN);

        /// Create renderer
        Renderer mainRenderer(&mainWindow, 5, settings.vsync ? SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC : SDL_RENDERER_ACCELERATED);
        mainWindow.setAspectRatio(16, 9);

        /// Create an EntityComponentSystem
        EntityComponentSystem entitySystem;

        ///
        /// ECS and text rendering demo
        ///

        Font font;
        int ptsizes[3] = {2, 24, 36};
        font.load("Orkney Regular.ttf", &ptsizes[0]);
        font.init("Orkney Regular.ttf");

        Entity gameObject(&entitySystem);
        gameObject.SetName("Test Entity");
        gameObject.AttachComponent<Text>();

        targetText = gameObject.GetComponent<Text>();
        if (targetText != nullptr)
        {
            targetText->setText("FPS: 0");
            targetText->setColor(colours::BLUE);
            //targetText->setBox(true);
            targetText->setOutline(1);
            targetText->setBackgroundColor(colours::GREEN);
            targetText->textToTexture(mainRenderer, &font);
            mainRenderer.Register(targetText);
        }

        gameObject.AttachComponent<Text>();
        vector<Text*> compList = gameObject.GetComponents<Text>();
        if (!compList.empty() && compList.size() > 1)
        {
            compList[1]->setColor(colours::RED);
            compList[1]->setText("Current master mute key is " + (string)SDL_GetKeyName(currentKey));
            compList[1]->textToTexture(mainRenderer, &font, 36);
            mainText = compList[1];
            mainRenderer.Register(compList[1]);
        }

        ///
        ///  ECS + Image + Texture demo
        ///

        Image testImg;
        testImg.LoadAndInit("sprite_test.png", mainRenderer, SDL_GetWindowPixelFormat(mainWindow.getWindow()));

        Entity other(&entitySystem);
        other.AttachComponent<Texture>();

        Texture* tex = other.GetComponent<Texture>();
        if (tex != nullptr)
        {
            tex->SetSource(&testImg);
            tex->position.x = (float)(mainRenderer.GetWidth() / 2);
            tex->position.y = (float)(mainRenderer.GetHeight() / 2);
            mainRenderer.Register(tex);
            /// Grayscale effect
            testImg.ApplyEffect([] (SDL_Color c, SDL_Point p) {
                Uint8 grayscale = (Uint8)(((float)c.r * 0.3f) + ((float)c.g * 0.59f) + ((float)c.b * 0.11f));
                c = Colour(grayscale, grayscale, grayscale, c.a);
                return c;
            });
        }

        ///
        /// Input handling demo
        ///

        InputContext mainContext;
        KeyboardHandler& keyboard = *mainContext.AddHandler<KeyboardHandler>();

        keyboard.AddAction("TOGGLE MUTE", *KeyAction);
        keyboard.Bind("TOGGLE MUTE", SDLK_m);
        keyboard.AddBindlessAction(*GetKey);

        MouseHandler& mouse = *mainContext.AddHandler<MouseHandler>();

        mouse.AddAction("mouseclick", *MouseClickAction);
        mouse.AddAction("scroll", *MouseScrollAction);
        mouse.Bind("mouseclick", MOUSE_BUTTON_LEFT);
        mouse.Bind("scroll", MOUSE_WHEEL);

        InputController mainInput;

        mainInput.AddContext("main", &mainContext);

        SDL_Event e;

        ///
        /// Audio demo
        ///

        AudioClip sound;
        AudioSource source;
        AudioBus sfx;

        SoundStream.Link(&master);
        SoundStream.Play("test_stream.wav", 0.5f, -1);

        /// Source goes into sfx bus
        source.Link(&sfx);
        /// The sfx bus goes into the master bus
        sfx.Link(&master);
        sfx.SetVolume(0);
        if (!sound.load("test_audio.wav"))
        {
            SDL_Log("Error loading sound! Mix_Error: %s", Mix_GetError());
        }
        else
        {
            source.Play(&sound, 0.1f, -1);
        }

        ///
        /// Init timing stuff before we start the main loop
        ///

        Timer fpsTimer;
        fpsTimer.start();
        int countedFrames = 0;

        /// Initialise the global delta time and FPS controller
        delta.init(settings);

        while (!quit)
        {
            /// Input handling phase
            while (SDL_PollEvent(&e) != 0)
            {
                mainWindow.handle_events(e);
                if (e.type == SDL_QUIT || (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE))
                {
                    quit = true;
                    break;
                }
                mainInput.HandleEvent(e);
            }

            /// Logic update phase
            if (volume_change)
            {
                source.SetPanning(panning);
                volume_change = false;
            }

            sfx.FadeIn(delta.time(), 3.0f);
            /// Demo dynamic key binding
            if (update_binding)
            {
                /// Rebind the key
                keyboard.Bind("TOGGLE MUTE", currentKey);
                update_binding = false;
            }

            entitySystem.UpdateComponents();

            /// Rendering phase
            SDL_RenderClear(mainRenderer.GetRendererSDL());

            vector<Text*> handyComponents = gameObject.GetComponents<Text>();
            if (!handyComponents.empty())
            {
                for (int i = 0, counti = handyComponents.size(); i < counti; i++)
                {
                    handyComponents[i]->position.x = (handyComponents[i]->getWidth() / 2) + posx;
                    handyComponents[i]->position.y = (handyComponents[i]->getHeight() / 2) + (i * 50) + posy;
                }
                if (fpsTimer.getTicks() > 250)
                {
                    handyComponents[0]->setText("FPS: " + ToString((int)(countedFrames / (fpsTimer.getTicks() / 1000.0f))));
                    countedFrames = 0;
                    fpsTimer.start();
                }
            }
            mainRenderer.RenderPresent(true);

            mainRenderer.SetDrawColour(colours::RED);
            SDL_Rect viewrect = mainWindow.getViewportRect();
            viewrect.x = 0;
            viewrect.y = 0;
            SDL_RenderDrawRect(mainRenderer.GetRendererSDL(), &viewrect);

            mainRenderer.SetDrawColour(colours::BLACK);
            SDL_RenderPresent(mainRenderer.GetRendererSDL());

            /// Update timer and FPS count
            countedFrames++;
            delta.update();
        }
    }
    TerminateOssium();
    return 0;
}
