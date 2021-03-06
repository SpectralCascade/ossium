#ifdef OSSIUM_EDITOR
#include "editorwindow.h"
#endif
#include "editorserializer.h"
#include "../../Core/coremaths.h"

using namespace std;
using namespace Ossium::Editor;

namespace Ossium
{

    void EditorSerializer::_InitEditorSerializer(EditorWindow* window)
    {
        gui = window;
    }

    string EditorSerializer::SerializeProperty(const char* type, const char* name, int attribute, string& property)
    {
#ifdef OSSIUM_EDITOR
        if (!(attribute & ATTRIBUTE_HIDDEN))
        {
            gui->BeginHorizontal();
            gui->TextLabel(string(name), EditorStyle::StandardText);
            gui->Tab(100);
            string data = attribute & ATTRIBUTE_FILEPATH ? gui->FilePathField(property) : gui->TextField(property);
            gui->EndHorizontal();
            dirty |= data != property;
            return data;
        }
#endif // OSSIUM_EDITOR
        return property;
    }

    string EditorSerializer::SerializeProperty(const char* type, const char* name, int attribute, bool& property)
    {
#ifdef OSSIUM_EDITOR
        if (!(attribute & ATTRIBUTE_HIDDEN))
        {
            gui->BeginHorizontal();
            gui->TextLabel(string(name), EditorStyle::StandardText);
            gui->Tab(100);
            bool data = gui->Toggle(property);
            gui->EndHorizontal();
            dirty |= data != property;
            return data ? "1" : "0";
        }
#endif // OSSIUM_EDITOR
        return Utilities::ToString(property);
    }

    string EditorSerializer::SerializeProperty(const char* type, const char* name, int attribute, Vector2& property)
    {
#ifdef OSSIUM_EDITOR
        if (!(attribute & ATTRIBUTE_HIDDEN))
        {
            gui->BeginHorizontal();
            gui->TextLabel(string(name), EditorStyle::StandardText);
            gui->Tab(100);
            Vector2 data = Vector2(Utilities::ToFloat(gui->TextField(Utilities::ToString(property.x))), 0);
            gui->Tab(50);
            data.y = Utilities::ToFloat(gui->TextField(Utilities::ToString(property.y)));
            gui->EndHorizontal();
            dirty |= data != property;
            return Utilities::ToString(data);
        }
#endif // OSSIUM_EDITOR
        return Utilities::ToString(property);
    }

    void EditorSerializer::ClearDirtyFlag()
    {
        dirty = false;
    }

    // Returns true if any properties have been modified since the last ClearDirtyFlag() call.
    bool EditorSerializer::IsDirty()
    {
        return dirty;
    }

}
