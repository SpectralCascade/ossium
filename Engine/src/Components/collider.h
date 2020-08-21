#ifndef COLLIDER_H
#define COLLIDER_H

#include "../Core/physics.h"
#include "../Core/component.h"

namespace Ossium
{

    struct PhysicsBodySchema : public Schema<PhysicsBodySchema, 20>
    {
        DECLARE_BASE_SCHEMA(PhysicsBodySchema, 20);

    protected:
        // TODO: make getters and setters for these members as appropriate.

        // Fixture definition variables
        M(float, density) = 1.0f;
        M(float, friction) = 0.3f;
        M(float, restitution) = 0.5f;
        M(bool, sensor) = false;

        // TODO: header attribute (will need to introduce string attributes).

        // Body definition variables
        M(b2BodyType, bodyType) = b2_dynamicBody;
        M(bool, bullet) = false;
        M(bool, allowSleep) = true;
        M(bool, startAwake) = true;
        M(bool, fixedRotation) = false;
        M(float, gravityScale) = 1.0f;
        M(float, linearDamping) = 0.5f;
        M(float, angularDamping) = 0.5f;

        // Always protected
        M(Vector2, initialLinearVelocity) = { 0.0f, 0.0f };
        M(float, initialAngularVelocity) = 0.0f;

    };

    class PhysicsBody : public Component, public PhysicsBodySchema
    {
    public:
        CONSTRUCT_SCHEMA(Component, PhysicsBodySchema);
        DECLARE_COMPONENT(Component, PhysicsBody);

        void OnLoadFinish();

        void OnDestroy();

        /// Updates position and rotation transform based on the Box2D body.
        void UpdatePhysics();

        // TODO: Make private, add getters and setters?
        b2Body* body = nullptr;

        // TODO: list of fixtures perhaps, or a dedicated component?
        b2Fixture* fixture = nullptr;

    private:
    #ifdef OSSIUM_EDITOR
        b2BodyType _editor_oldBodyType = b2_dynamicBody;
    #endif // OSSIUM_EDITOR

    };

    class Collider : public GraphicComponent
    {
    public:
        DECLARE_ABSTRACT_COMPONENT(GraphicComponent, Collider);

        virtual const b2Shape& GetShape() = 0;

    };

}

#endif // COLLIDER_H
