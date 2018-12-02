#pragma once

#ifndef ECS_H
#define ECS_H

#include <vector>
#include <string>
#include <unordered_map>
#include <SDL2/SDL.h>

#include "transform.h"

using namespace std;

namespace ossium
{

    typedef Uint32 ComponentType;

    namespace ecs
    {
        /// This is a compile-time construct designed to provide compile-time type identification
        /// as an efficient alternative to RTTI
        class ComponentRegistry
        {
        private:
            static ComponentType nextTypeIdent;
            /// The type identifier for this registered component
            ComponentType typeIdent;

        public:
            ComponentRegistry()
            {
                typeIdent = nextTypeIdent;
                /// Now increment nextTypeIdent for when we register another type
                nextTypeIdent++;
            }

            /// The type identifier never changes once set for a type, hence const
            const ComponentType getIdent()
            {
                return typeIdent;
            }

        };
    }

    /// Declares a component type
    /// Add this to the end of any class you wish to register as a component
    #define DECLARE_COMPONENT public: static ecs::ComponentRegistry __ecs_entry_
    /// Adds the component type to the registry by static instantiation
    /// Add this to the class definition file of a component that uses DECLARE_COMPONENT
    #define REGISTER_COMPONENT(name) ecs::ComponentRegistry name::__ecs_entry_

    /// Compile time constant return type
    template<class T>
    ComponentType getComponentTypeIdent()
    {
        return T::__ecs_entry_.getIdent();
    }

    class Component;

    /// Nothing should inherit from Entity.
    /// I would use the 'final' keyword to ensure this
    /// but Code::Blocks autocomplete stops working properly!
    class Entity
    {
    public:
        Entity();
        ~Entity();

        /// Instantiates and attaches a component to this entity
        template<class T>
        void AttachComponent()
        {
            T* component = new T();
            component->entity = this;
            component->OnCreate();
            auto itr = components.find(getComponentTypeIdent<T>());
            if (itr != components.end())
            {
                itr->second.push_back(component);
            }
            else
            {
                vector<Component*> component_vector;
                component_vector.push_back(component);
                components.insert({getComponentTypeIdent<T>(), component_vector});
            }
        }

        /// Destroys and removes first found instance of a component attached to this entity
        template<class T>
        void RemoveComponent()
        {
            auto itr = components.find(getComponentTypeIdent<T>());
            if (itr != components.end() && !itr->second.empty())
            {
                itr->second[0]->OnDestroy();
                delete itr->second[0];
                itr->second[0] = nullptr;
                itr->second.erase(itr->second.begin());
            }
        }

        /// Returns a pointer the first found instance of a component attached
        /// to this entity of type T
        template <class T>
        T* GetComponent()
        {
            auto itr = components.find(getComponentTypeIdent<T>());
            if (itr != components.end() && !itr->second.empty())
            {
                return reinterpret_cast<T*>(itr->second[0]);
            }
            return nullptr;
        }

        /// Returns a vector of pointers to all component instances of a given type
        /// attached to this entity
        template <class T>
        vector<T*> GetComponents()
        {
            auto itr = components.find(getComponentTypeIdent<T>());
            if (itr != components.end())
            {
                return reinterpret_cast<vector<T*>>(itr->second);
            }
            // Return an empty vector
            vector<T*> none;
            return none;
        }

        template <class T>
        bool HasComponent()
        {
            return GetComponent<T>() != nullptr;
        }

        /// Returns this entity's ID
        const int GetID();

        /// Name of this entity (not necessarily unique)
        string name;

        /// Transform data for this entity
        Transform transform;

    private:
        /// Hashtable of components attached to this entity by type
        unordered_map<ComponentType, vector<Component*>> components;

        /// The ID of this entity (unique)
        int id;

        /// Incremental id
        static int nextId;

    };

    /// Base class for all components
    class Component
    {
    public:
        friend class Entity;

        Component();
        virtual ~Component();

    protected:
        /// Effectively replace the constructor and destructor
        virtual void OnCreate();
        virtual void OnDestroy();

        /// Called when the parent entity is spawned
        virtual void OnSpawn();

        /// Each frame this method is called
        virtual void Update();

        /// Pointer to the entity that this component is attached to
        Entity* entity = nullptr;

    private:
        #ifdef DEBUG
        bool onDestroyCalled;
        #endif // DEBUG

    };

}

#endif // ECS_H
