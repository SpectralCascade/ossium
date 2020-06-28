#include "demo_window_docking.h"
#include "simple_demo_window.h"
#include "layout_diagram.h"
#include "../Core/contextmenu.h"

namespace Ossium::Editor
{

    void DemoDockingWindow::OnGUI()
    {

        if (Rect(0, 0, viewport.w, viewport.h).Contains(InputState.mousePos))
        {
            LayoutDiagram::mouseHovered = this;
        }

        Space(20);

        BeginHorizontal();
        {
            TextLabel("<b>Window docking demo</b>");

            Space(20);

            NeuronClickableStyle style = NeuronStyles::NEURON_BUTTON_STYLE;
            style.hoverColor = Colors::RED;
            style.normalColor = Color(230, 0, 0);
            style.clickColor = Color(150, 0, 0);
            style.normalTextStyle.fg = Colors::WHITE;
            style.hoverTextStyle.fg = Colors::WHITE;
            style.clickTextStyle.fg = Color(100, 100, 100);

            if (Button("Test Dropdown"))
            {
                if (ContextMenu::GetMainInstance()->GetOptions().empty())
                {
                    ContextMenu::GetMainInstance()->SetOptions({
                        ContextMenu::Option("Beep", [] () { Logger::EngineLog().Info("Beep!"); }),
                        ContextMenu::Option("Boop", [] () { Logger::EngineLog().Info("Boop!"); }),
                        ContextMenu::Option("Wow", [] () { Logger::EngineLog().Info("Wooooooowww"); })
                    });
                }
                ContextMenu::GetMainInstance()->Show(GetEditorLayout()->GetNativeWindow()->GetPosition() + Vector2(viewport.x, viewport.y) + GetLayoutPosition());
            }

            Space(20);

            if (Button("<b>X</b>", style, true, 12, 8))
            {
                Destroy();
            }
        }
        EndHorizontal();

        Space((viewport.h / 2) - 64);

        BeginHorizontal();
        {
            Space((viewport.w / 2) - 32);

            if (Button("Dock Above"))
            {
                GetEditorLayout()->Add<DemoDockingWindow>(this, DockingMode::TOP);
            }
        }
        EndHorizontal();

        Space(5);

        BeginHorizontal();
        {
            Space((viewport.w / 2) - 64);

            if (Button("Dock Left"))
            {
                GetEditorLayout()->Add<DemoDockingWindow>(this, DockingMode::LEFT);
            }

            Space(20);

            if (Button("Dock Right"))
            {
                GetEditorLayout()->Add<DemoDockingWindow>(this, DockingMode::RIGHT);
            }
        }
        EndHorizontal();

        Space(5);

        BeginHorizontal();
        {
            Space((viewport.w / 2) - 32);

            if (Button("Dock Below"))
            {
                GetEditorLayout()->Add<DemoDockingWindow>(this, DockingMode::BOTTOM);
            }
        }
        EndHorizontal();

    }

}
