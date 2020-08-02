#include "editorcontroller.h"
#include "editorlayout.h"
#include "../Windows/ToolBar.h"
#include "contextmenu.h"
#include "project.h"

using namespace std;

namespace Ossium::Editor
{

    EditorController::EditorController(ResourceController* resources)
    {
        this->resources = resources;
        input = new InputController();
        mainLayout = new EditorLayout(this, "Ossium Editor");
        toolbar = mainLayout->Add<ToolBar>(DockingMode::TOP);
        mainLayout->GetNativeWindow()->OnCloseButton += [&] (Window& caller) {
            running = false;
        };
    }

    EditorController::~EditorController()
    {
        for (auto layout : layouts)
        {
            delete layout;
        }
        layouts.clear();
        delete mainLayout;
        delete input;
    }

    EditorController::MenuTool::MenuTool(string path, function<void()> onClick, function<bool()> isEnabled)
    {
        this->path = path;
        this->onClick = onClick;
        this->isEnabled = isEnabled;
    }

    void EditorController::AddCustomMenu(string menuPath, function<void()> onClick, function<bool()> isEnabled)
    {
        customMenuTools.push_back(MenuTool(menuPath, onClick, isEnabled));
    }

    EditorLayout* EditorController::CreateLayout()
    {
        EditorLayout* layout = new EditorLayout(this, "Ossium Editor");
        layouts.push_back(layout);
        return layout;
    }

    void EditorController::RemoveLayout(EditorLayout* layout)
    {
        if (layout != nullptr)
        {
            for (auto itr = layouts.begin(); itr != layouts.end(); itr++)
            {
                if (*itr == layout)
                {
                    layouts.erase(itr);
                    delete layout;
                    break;
                }
            }
        }
        if (layout == mainLayout)
        {
            Log.Warning("Attempted to remove main layout from an EditorController instance! Quitting editor...");
            running = false;
        }
    }

    bool EditorController::Update()
    {
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                running = false;
                break;
            }

            // Update the context menu
            ContextMenu::HandleInput(e);

            // Update native layout windows
            mainLayout->HandleEvent(e);
            for (auto layout : layouts)
            {
                layout->HandleEvent(e);
            }

            // Handle standard input events.
            input->HandleEvent(e);
        }

        // Update the GUI
        mainLayout->Update();
        for (auto layout : layouts)
        {
            layout->Update();
        }

        ContextMenu::GetMainInstance(resources)->Update();

        if (toolbar->ShouldQuit())
        {
            running = false;
        }

        return running;
    }

    Project* EditorController::GetProject()
    {
        return loadedProject;
    }

    Project* EditorController::CreateProject()
    {
        CloseProject();
        loadedProject = new Project();
        return loadedProject;
    }

    Project* EditorController::OpenProject(string path)
    {
        CloseProject();
        loadedProject = new Project();
        loadedProject->Load(path);
        for (ListedScene& scene : loadedProject->openScenes)
        {
            if (scene.loaded)
            {
                // TODO: consider how the Scene is loaded/initialised; it is dependant on a renderer, which is limited to an individual window.
                // This isn't ideal, potentially scene loading can be done independently to avoid limiting the scene view to the main layout window?
                resources->LoadAndInit<Scene>(scene.path, mainLayout->GetServices());
            }
            else
            {
                resources->Free<Scene>(scene.path);
            }
        }
        loadedProject->SetPath(Utilities::StripFilename(path));
        return loadedProject;
    }

    void EditorController::CloseProject()
    {
        if (loadedProject != nullptr)
        {
            delete loadedProject;
            loadedProject = nullptr;
        }
    }

    ResourceController* EditorController::GetResources()
    {
        return resources;
    }

    InputController* EditorController::GetInput()
    {
        return input;
    }

    EditorLayout* EditorController::GetMainLayout()
    {
        return mainLayout;
    }

}