#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <SDL.h>
#include <SDL_ttf.h>

#include "vector.h"
#include "ecs.h"
#include "colours.h"
#include "primitives.h"
#include "metarect.h"
#include "font.h"

using namespace std;

namespace Ossium
{

    inline namespace structs
    {
        struct Rect;
        struct Point;
    }

    enum TextRenderModes
    {
        RENDERTEXT_SOLID = 0,
        RENDERTEXT_SHADED,
        RENDERTEXT_BLEND
    };

    inline namespace graphics
    {

        /// Forward declare Texture
        class Texture;

        /// This class wraps an SDL_Texture
        class Image
        {
        public:
            Image();
            ~Image();

            friend class Texture;

            /// Destroys the image, freeing it from memory. Does not modify the temporary SDL_Surface
            void Free();

            /// Load an image and returns true if it was successful
            bool Load(string guid_path, int* loadArgs = NULL);

            /// Creates an image with a text string
            bool CreateFromText(Renderer& renderer, Font& fontToUse, string text, int pointSize = 12, SDL_Color colour = colours::RED,
                                int hinting = 0, int kerning = 0, int outline = 0, int style = 0, int renderMode = 0, SDL_Color bgColour = colours::BLACK);

            /// Post-load texture initialisation; pass the window pixel format if you wish to manipulate pixel data.
            /// You MUST call this method after successfully calling Load() if you wish to render the image to the screen.
            /// If cache is set to true, the original image is stored in memory as an SDL_Surface for quick initialisation without loading the image again
            /// (in addition to the modifiable SDL_Texture in video memory)! This could be useful if you want to remove all applied effects on the image frequently,
            /// as long as your target system has adequate memory capacity for storing the original image. Leave this set to false unless you know what you're doing!
            bool Init(Renderer& renderer, Uint32 windowPixelFormat = SDL_PIXELFORMAT_UNKNOWN, bool cache = false);

            /// Shorthand method
            bool LoadAndInit(string guid_path, Renderer& renderer, Uint32 windowPixelFormat = SDL_PIXELFORMAT_UNKNOWN, bool cache = false);

            /// Returns true if the texture is not NULL
            bool Initialised();

            /// Returns the width of the image
            int GetWidth();
            /// Returns the height of the image
            int GetHeight();

            /// Applies a pixel manipulation function to the image. The argument function accepts pixel data as an SDL_Color
            /// in addition to an SDL_Point which indicates which pixel is being modified relative to the top left of the image.
            /// It works by iterating over each pixel in the source image and giving your function relevant information. The function must return an SDL_Color value which is what the pixel
            /// will be set to. Note that the image must be initialised with the window's pixel format to apply pixel manipulation effects!
            /// Also note that this doesn't do it's work on the GPU, so be wary of using it frequently as it's pretty expensive
            /// Returns false on failure.
            template<class Func>
            bool ApplyEffect(Func f)
            {
                if (LockPixels())
                {
                    Uint32* pixelArray = reinterpret_cast<Uint32*>(pixels);
                    SDL_PixelFormat* pixelFormat = SDL_AllocFormat(format);
                    if (pixelFormat == NULL)
                    {
                        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not allocate pixel format to apply effect! SDL_Error: %s", SDL_GetError());
                        return false;
                    }
                    SDL_Color pixelData;
                    SDL_Point pixelPos;
                    for (int i = 0, counti = (pitch / 4) * height; i < counti; i++)
                    {
                        pixelData = ConvertToColour(pixelArray[i], pixelFormat);
                        pixelPos.x = i % width;
                        pixelPos.y = i / width;
                        SDL_Color outputColour = f(pixelData, pixelPos);
                        pixelArray[i] = SDL_MapRGBA(pixelFormat, outputColour.r, outputColour.g, outputColour.b, outputColour.a);
                    }
                    UnlockPixels();
                    SDL_FreeFormat(pixelFormat);
                    return true;
                }
                return false;
            };

        private:
            NOCOPY(Image);

            /// Locks the texture so the raw pixels may be modified
            bool LockPixels();
            /// Unlocks the texture so the raw pixels can no longer be modified
            bool UnlockPixels();

            /// The image prior to being converted to an SDL_Texture
            SDL_Surface* tempSurface;

            /// A representation of the image in video memory
            SDL_Texture* texture;

            /// Outline texture (used purely for rendered text)
            SDL_Texture* outlineTexture;

            /// Image dimensions
            int width;
            int height;

            /// The pixel format
            Uint32 format;
            /// Pixel data; NULL unless you have locked the pixels in video memory
            void* pixels;
            /// 'Width' of the image as laid out in memory, in bytes
            int pitch;

        };

        /// This class is used for rendering an image
        class Texture : public Graphic, public Component, public MetaRect<Texture>
        {
        public:
            DECLARE_COMPONENT(Texture);

            Texture();
            Texture(const Texture& src);
            virtual ~Texture();

            /// Sets the alpha blending mode
            void SetBlendMode(SDL_BlendMode blend);

            /// Sets alpha modulation
            void SetAlphaMod(Uint8 a);
            /// Sets colour modulation
            void SetColourMod(Uint8 r, Uint8 g, Uint8 b);
            /// Sets both colour and alpha modulation
            void SetMod(SDL_Color mod);

            /// Returns the colour and alpha modulation values for this texture
            SDL_Color GetMod();

            /// Inherited Graphic::Render() method
            virtual void Render(Renderer& renderer);

            /// Sets the source image this texture should use
            void SetSource(Image* src);
            /// Returns a pointer to the source image
            Image* GetSource();

            /// Sets the rendered position of the texture (from the centre)
            void SetPosition(float x, float y);
            /// Sets the rotation of the texture
            void SetRotation(float rot_angle);
            /// Sets the point at which the texture is rotated as percentages of the destination width and height
            void SetRelativeOrigin(float xpercent, float ypercent);
            /// Sets the width as a percentage of the source image width
            void SetRenderWidth(float percent);
            /// Sets the height as a percentage of the source image height
            void SetRenderHeight(float percent);
            /// Sets the flip mode of the texture; a texture can be flipped horizontally, vertically, or not at all
            void SetFlip(SDL_RendererFlip flipMode);
            /// Sets the clip rect area
            void SetClip(int x, int y, int w = 0, int h = 0);


            /// Gets the source image width
            int GetSourceWidth();
            /// Gets the source image height
            int GetSourceHeight();
            /// Gets the width as a percentage of the source image width
            float GetRenderWidth();
            /// Ditto, but as a percentage of the source image height
            float GetRenderHeight();
            /// Gets the flip mode of the texture; a texture can be flipped horizontally, vertically, or not at all
            SDL_RendererFlip GetFlip();

        private:
            /// The source image that this texture renders a copy of
            Image* source;

            /// The area of the source image that should be rendered
            SDL_Rect clip;

            /// Should this texture be flipped vertically, horizontally, or not at all?
            SDL_RendererFlip flip;

            /// Colour and alpha modulation values. These are applied whenever the Render() method is called
            SDL_Color modulation;

            /// The blending mode for this texture. Applied whenever the Render() method is called
            SDL_BlendMode blending;

        };

    }

}

#endif // TEXTURE_H
