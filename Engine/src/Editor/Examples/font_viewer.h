#ifndef FONT_VIEWER_H
#define FONT_VIEWER_H

#include "../Components/editorview.h"

namespace Ossium::Editor
{

    class FontViewer : public EditorWindow
    {
    protected:
        void OnGUI();

        SDL_Color color = Colors::BLACK;

        string colorText = "000000";

        string inputFontPath = "assets/Orkney Regular.ttf";

        string currentFontPath = "assets/Orkney Regular.ttf";

        float scale = 1.0f;

    };

}

#endif // FONT_VIEWER_H