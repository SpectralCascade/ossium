/** COPYRIGHT NOTICE
 *
 *  Ossium Engine
 *  Copyright (c) 2018-2020 Tim Lane
 *
 *  This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
 *
 *  2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
 *
 *  3. This notice may not be removed or altered from any source distribution.
 *
**/
#include <string>
#include <algorithm>
#include <filesystem>

#include "text.h"
#include "../Core/renderer.h"
#include "../Core/ecs.h"

namespace Ossium
{

    REGISTER_COMPONENT(Text);

    void Text::OnLoadFinish()
    {
        GraphicComponent::OnLoadFinish();
        // Check if the path is valid
        if (std::filesystem::exists(std::filesystem::path(font_guid)))
        {
            font = GetService<ResourceController>()->Get<Font>(font_guid, 72, 0, 0, 2048);
        }
        else
        {
            font = nullptr;
        }
        dirty = true;
    }

    void Text::Render(Renderer& renderer)
    {
        if (boxed)
        {
            Rect boxDest = Rect(
                GetTransform()->GetWorldPosition().x - boxPaddingWidth,
                GetTransform()->GetWorldPosition().y - boxPaddingHeight,
                layout.GetBounds().x + boxPaddingWidth,
                layout.GetBounds().y + boxPaddingHeight
            );
            boxDest.DrawFilled(renderer, backgroundColor);
        }

        if (font != nullptr)
        {
            /// TODO: figure out why we have to update every frame! Maybe the font atlas glyph cache breaks?
            layout.SetText(renderer, *font, text, applyMarkup);
            layout.Update(*font);

            layout.Render(renderer, *font, GetTransform()->GetWorldPosition());
        }
    }

}
