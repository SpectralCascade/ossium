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
#ifndef PHYSICS_H
#define PHYSICS_H

#include <Box2D/Box2D.h>
#include <functional>

#include "coremaths.h"
#include "services.h"

namespace Ossium
{

    inline float PTM(float pixels)
    {
        return pixels * 0.02f;
    }

    inline float MTP(float metres)
    {
        return metres * (1.0f / 0.02f);
    }

    inline Vector2 PTM(const Vector2& vecPixels)
    {
        return Vector2(PTM(vecPixels.x), PTM(vecPixels.y));
    }

    inline Vector2 MTP(const Vector2& vecMetres)
    {
        return Vector2(MTP(vecMetres.x), MTP(vecMetres.y));
    }

    inline namespace Physics
    {

        class OSSIUM_EDL OnRayHit : public b2RayCastCallback
        {
        public:
            typedef std::function<float32(b2Fixture*, const b2Vec2&, const b2Vec2&, float32)> RayHitCallback;

            OnRayHit(RayHitCallback callback);

            virtual ~OnRayHit() = default;

            float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction);

        private:
            RayHitCallback onRayHit;
        };

        class OSSIUM_EDL PhysicsWorld : public b2World, public Service<PhysicsWorld>
        {
        public:
            PhysicsWorld() : b2World({0.0f, -9.81f}) {};
            PhysicsWorld(Vector2 gravity) : b2World(gravity) {};

            /// Overloads that make it easier to perform raycasting and allow for use of Lambdas.
            void RayCast(const Ray& ray, b2RayCastCallback* callback, float distance = 100.0f);
            void RayCast(const Ray& ray, OnRayHit* callback, float distance = 100.0f);
            void RayCast(const Point& origin, const Point& endPoint, OnRayHit* callback);

            /// BaseService::PostUpdate() override, updates physics after the scenes have been updated.
            void PostUpdate();

            /// Freeze or unfreeze the physics world.
            void SetFrozen(bool freeze);
            bool IsFrozen();

            float timeStep = 1.0f / 60.0f;
            int velocityIterations = 6;
            int positionIterations = 2;

        private:
            bool frozen = false;

        };

    }

}

#endif // PHYSICS_H
