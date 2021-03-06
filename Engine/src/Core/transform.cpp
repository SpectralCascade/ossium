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
#include "transform.h"

using namespace std;

namespace Ossium
{

    REGISTER_COMPONENT(Transform);

    void Transform::RefreshData()
    {
        if (dirty)
        {
            Entity* parent = entity->GetParent();
            if (parent != nullptr)
            {
                worldPosition = position + parent->AddComponentOnce<Transform>()->GetWorldPosition();
                worldRotation = Rotation(b2Mul(rotation, parent->AddComponentOnce<Transform>()->GetWorldRotation()));
                worldScale = scale * parent->AddComponentOnce<Transform>()->GetWorldScale();
            }
            else
            {
                worldPosition = position;
                worldRotation = rotation;
                worldScale = scale;
            }
            dirty = false;
        }
    }

    void Transform::SetRelativeToParent(bool setRelative)
    {
        if (relative != setRelative)
        {
            relative = setRelative;
            SetDirty();
        }
    }

    void Transform::SetDirty()
    {
        if (!dirty)
        {
            dirty = true;
            vector<Transform*> children = entity->GetComponentsInChildren<Transform>();
            for (auto t : children)
            {
                t->SetDirty();
            }
        }
    }

    Point Transform::GetLocalPosition()
    {
        return position;
    }

    Rotation Transform::GetLocalRotation()
    {
        return rotation;
    }

    Vector2 Transform::GetLocalScale()
    {
        return scale;
    }

    void Transform::SetLocalPosition(Point p)
    {
        position = p;
        SetDirty();
    }

    void Transform::SetLocalRotation(Rotation r)
    {
        rotation = r;
        SetDirty();
    }

    void Transform::SetLocalScale(Vector2 s)
    {
        scale = s;
        SetDirty();
    }

    void Transform::SetLocal(Point p, Rotation r, Vector2 s)
    {
        position = p;
        rotation = r;
        scale = s;
        SetDirty();
    }

    Point Transform::GetWorldPosition()
    {
        if (!relative)
        {
            return position;
        }
        RefreshData();
        return worldPosition;
    }

    Rotation Transform::GetWorldRotation()
    {
        if (!relative)
        {
            return rotation;
        }
        RefreshData();
        return worldRotation;
    }

    Vector2 Transform::GetWorldScale()
    {
        if (!relative)
        {
            return scale;
        }
        RefreshData();
        return worldScale;
    }

    void Transform::SetWorldPosition(Point p)
    {
        position += p - GetWorldPosition();
        SetDirty();
    }

    void Transform::SetWorldRotation(Rotation r)
    {
        // TODO
        rotation = r;
        SetDirty();
    }

    void Transform::SetWorldScale(Vector2 s)
    {
        scale += s - GetWorldScale();
        SetDirty();
    }

    void Transform::SetWorld(Point p, Rotation r, Vector2 s)
    {
        position += p - GetWorldPosition();
        rotation = r; // TODO
        scale += s - GetWorldScale();
        SetDirty();
    }

}
