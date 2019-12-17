#include <cstdio>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <vector>

#include "funcutils.h"
#include "stringconvert.h"
#include "ecs.h"
#include "delta.h"

using namespace std;

namespace Ossium
{

    string GetComponentName(Uint32 id)
    {
        return TypeSystem::TypeFactory<BaseComponent, ComponentType>::GetName(id);
    }

    ComponentType GetComponentType(string name)
    {
        return TypeSystem::TypeFactory<BaseComponent, ComponentType>::GetId(name);
    }

    Entity::Entity(EntityComponentSystem* entity_system, Entity* parent)
    {
        controller = entity_system;
        string name = "Entity";
        if (parent != nullptr)
        {
            self = controller->entityTree.insert(name, this, parent->self);
        }
        else
        {
            self = controller->entityTree.insert(name, this);
        }
        controller->entities[self->id] = self;
        /// Set the name again, using the generated id
        name = "Entity[" + Ossium::ToString(self->id) + "]";
        SetName(name);
    }

    Entity* Entity::Clone()
    {
        Entity* entityCopy = new Entity(controller, self->parent != nullptr ? self->parent->data : nullptr);
        entityCopy->self->name = self->name + " (copy)";
        for (auto i = components.begin(); i != components.end(); i++)
        {
            vector<BaseComponent*> copiedComponents;
            for (auto itr = i->second.begin(); itr != i->second.end(); itr++)
            {
                BaseComponent* copyComponent = (*itr)->Clone();
                copyComponent->entity = entityCopy;
                copyComponent->OnClone(*itr);
                copiedComponents.push_back(copyComponent);
            }
            entityCopy->components.insert({i->first, copiedComponents});
        }
        return entityCopy;
    }

    Entity::~Entity()
    {
        /// Destroy all components
        for (auto itr = components.begin(); itr != components.end(); itr++)
        {
            for (unsigned int i = 0, counti = itr->second.empty() ? 0 : itr->second.size(); i < counti; i++)
            {
                if (itr->second[i] != nullptr)
                {
                    vector<BaseComponent*>& ecs_components = controller->components[itr->second[i]->GetType()];
                    for (auto j = ecs_components.begin(); j != ecs_components.end(); j++)
                    {
                        if (*j == itr->second[i])
                        {
                            ecs_components.erase(j);
                            break;
                        }
                    }
                    /// Now remove the component pointer from this entity's components hash and delete it
                    itr->second[i]->OnDestroy();
                    delete itr->second[i];
                    itr->second[i] = nullptr;
                }
            }
            itr->second.clear();
        }
        components.clear();
        /// Clean up all children
        controller->entityTree.remove(self);
        controller->entities.erase(self->id);
    }

    vector<BaseComponent*>& Entity::GetComponents(ComponentType compType)
    {
        return components[compType];
    }

    const int Entity::GetID()
    {
        return self->id;
    }

    string Entity::GetReferenceID()
    {
        return Utilities::ToString(self->id);
    }

    void Entity::SetName(string name)
    {
        self->name = name;
    }

    string Entity::GetName()
    {
        return self->name;
    }

    void Entity::MapReference(string ident, void** ptr)
    {
        controller->serialised_pointers[ident].insert(ptr);
    }

    Entity* Entity::GetParent()
    {
        /// Check if this is the root entity, or the parent is the root entity
        /// We can assume self is never null as self is set when the entity is added to the entity tree
        if (self->parent == nullptr || self->name == "" || self->parent->name == "")
        {
            return nullptr;
        }
        else
        {
            return self->parent->data;
        }
    }

    Entity* Entity::Find(string name)
    {
        Node<Entity*>* node = controller->entityTree.find(name);
        return node != nullptr ? node->data : nullptr;
    }

    Entity* Entity::Find(string name, Entity* parent)
    {
        Node<Entity*>* node = controller->entityTree.find(name, parent->self);
        return node != nullptr ? node->data : nullptr;
    }

    Entity* Entity::CreateChild()
    {
        return controller->CreateEntity(this);
    }

    void Entity::SetParent(Entity* parent)
    {
        self->SetParent(parent != nullptr ? parent->self : nullptr);
    }

    void Entity::FromString(string& str)
    {
        JSON data(str);

        auto entity_itr = data.find("Name");
        if (entity_itr != data.end())
        {
            SetName(entity_itr->second);
        }
        else
        {
            Logger::EngineLog().Error("Failed to get entity name!");
        }

        entity_itr = data.find("Components");
        if (entity_itr != data.end())
        {
            JSON components_data(entity_itr->second);

            for (auto component : components_data)
            {
                ComponentType compType = 0;
                /// TODO: map compType to the type id specified in a lookup table within the JSON data
                /// to ensure component type ids are correct between versions/devices (due to static instantiation order uncertainty).
                compType = GetComponentType(component.first);
                if (!TypeSystem::TypeRegistry<BaseComponent>::IsValidType(compType))
                {
                    Logger::EngineLog().Error("Failed to add component of type \"{0}\" [{1}] due to invalid type!", component.first, compType);
                    continue;
                }
                //Logger::EngineLog().Info("Creating component of type \"{0}\" [{1}]", component.first, compType);
                vector<JString> componentData = component.second.ToArray();
                vector<BaseComponent*>& compsOfType = components[compType];
                unsigned int totalComponents = compsOfType.empty() ? 0 : compsOfType.size();
                for (unsigned int i = 0, counti = componentData.empty() ? 0 : componentData.size(); i < counti; i++)
                {
                    BaseComponent* comp = nullptr;
                    if (i >= totalComponents)
                    {
                        comp = TypeSystem::TypeFactory<BaseComponent, ComponentType>::Create(compType, (void*)this);
                        if (comp == nullptr)
                        {
                            Logger::EngineLog().Error("Failed to add component of type \"{0}\" [{1}] to entity during Entity::FromString()!", component.first, compType);
                            continue;
                        }
                        else
                        {
                            //Logger::EngineLog().Info("Created component of type \"{0}\" [{1}].", GetComponentName(comp->GetType()), comp->GetType());
                            totalComponents++;
                        }
                    }
                    else
                    {
                        comp = compsOfType[i];
                    }
                    comp->OnLoadStart();
                    comp->FromString(componentData[i]);
                }
            }

        }
        else
        {
            Logger::EngineLog().Error("Invalid JSON string input during entity FromString() call!");
        }
    }

    string Entity::ToString()
    {
        JSON data;
        JSON json_components;
        for (auto itr : components)
        {
            vector<JString> component_array;
            for (int i = 0, counti = itr.second.empty() ? 0 : itr.second.size(); i < counti; i++)
            {
                component_array.push_back(itr.second[i]->ToString());
            }
            json_components[GetComponentName((ComponentType)itr.first)] = Utilities::ToString(component_array);
        }
        data["Name"] = GetName();
        data["Parent"] = Utilities::ToString(self->parent != nullptr && self->parent->data != nullptr ? self->parent->id : -1);
        data["Components"] = json_components.ToString();
        return data.ToString();
    }

    void Entity::Destroy(bool immediate)
    {
        controller->DestroyEntity(this, immediate);
    }

    ///
    /// BaseComponent
    ///

    BaseComponent::~BaseComponent()
    {
    }

    void BaseComponent::OnCreate()
    {
    }

    void BaseComponent::OnDestroy()
    {
    }

    void BaseComponent::OnLoadStart()
    {
    }

    void BaseComponent::OnLoadFinish()
    {
    }

    void BaseComponent::OnClone(BaseComponent* src)
    {
    }

    void BaseComponent::Update()
    {
    }

    BaseComponent::BaseComponent()
    {
    }

    BaseComponent::BaseComponent(const BaseComponent& copySource)
    {
        /// Don't copy anything - we only care about child class values
        /// The entity reference should be constant once created
    }

    Entity* BaseComponent::GetEntity()
    {
        return entity;
    }

    string BaseComponent::GetReferenceID()
    {
        ComponentType compType = GetType();
        Entity* parentEntity = GetEntity();
        vector<BaseComponent*>& entComps = parentEntity->GetComponents(compType);
        for (unsigned int i = 0, counti = entComps.empty() ? 0 : entComps.size(); i < counti; i++)
        {
            if (entComps[i] == this)
            {
                return Utilities::ToString(parentEntity->GetID())
                         + ":" + GetComponentName(compType) + ":" + Utilities::ToString(i);
            }
        }
        return string("null");
    }

    ///
    /// EntityComponentSystem
    ///

    EntityComponentSystem::EntityComponentSystem(ServicesProvider* services)
    {
        if (services == nullptr)
        {
            Logger::EngineLog().Error("ServicesProvider argument cannot be NULL when creating an ECS instance!");
            DEBUG_ASSERT(services != nullptr, "ServicesProvider argument cannot be NULL when creating an ECS instance!");
            throw;
        }
        servicesProvider = services;
        components = new vector<BaseComponent*>[TypeSystem::TypeRegistry<BaseComponent>::GetTotalTypes()];
    }

    void EntityComponentSystem::UpdateComponents()
    {
        for (unsigned int i = 0, counti = TypeSystem::TypeRegistry<BaseComponent>::GetTotalTypes(); i < counti; i++)
        {
            for (auto j = components[i].begin(); j != components[i].end(); j++)
            {
                /// We shouldn't need to check if the component is null - if it is we have bigger problems!
                (*j)->Update();
            }
        }
    }

    Entity* EntityComponentSystem::CreateEntity()
    {
        Entity* created = new Entity(this);
        return created;
    }

    Entity* EntityComponentSystem::CreateEntity(Entity* parent)
    {
        Entity* created = new Entity(this, parent);
        return created;
    }

    void EntityComponentSystem::DestroyEntity(Entity* entity, bool immediate)
    {
        if (entity != nullptr)
        {
            if (entity->controller == this)
            {
                if (immediate)
                {
                    delete entity;
                }
                else
                {
                    pendingDestruction.push_back(entity);
                }
            }
            else
            {
                Logger::EngineLog().Warning("Attempted to destroy entity but it is not managed by this entity component system instance!");
            }
        }
        else
        {
            Logger::EngineLog().Warning("Attempted to destroy entity but it was already destroyed.");
        }
    }

    void EntityComponentSystem::DestroyPending()
    {
        for (auto entity : pendingDestruction)
        {
            delete entity;
        }
        pendingDestruction.clear();
    }

    void EntityComponentSystem::Clear()
    {
        /// Delete all entities
        vector<Node<Entity*>*>& entities = entityTree.getFlatTree();
        for (auto i = entities.begin(); i != entities.end(); i++)
        {
            if (*i != nullptr && (*i)->data != nullptr)
            {

                delete (*i)->data;
                (*i)->data = nullptr;
            }
        }
        /// Now we can safely remove all nodes from the tree and remove all components
        entityTree.clear();
        for (unsigned int i = 0, counti = TypeSystem::TypeRegistry<BaseComponent>::GetTotalTypes(); i < counti; i++)
        {
            /// No need to delete components as they are deleted when their parent entity is destroyed
            components[i].clear();
        }
    }

    unsigned int EntityComponentSystem::GetTotalEntities()
    {
        return entityTree.size();
    }

    string EntityComponentSystem::ToString()
    {
        JSON serialised;
        for (auto mappedEntity : entities)
        {
            Node<Entity*>* entity = mappedEntity.second;
            if (entity != nullptr && entity->data != nullptr)
            {
                serialised[Utilities::ToString(entity->id)] = entity->data->ToString();
            }
        }
        return serialised.ToString();
    }

    void EntityComponentSystem::FromString(string& str)
    {
        /// TODO: don't clear..?
        Clear();
        JSON serialised(str);
        /// Map of entities that need to be nested under a parent entity
        vector<pair<Entity*, int>> parentMap;
        for (auto itr : serialised)
        {
            if (!IsInt(itr.first))
            {
                Logger::EngineLog().Error("Failed to load entity due to invalid ID '{0}'!", itr.first);
                continue;
            }
            int id = ToInt(itr.first);
            if (id >= entityTree.GetGeneration())
            {
                /// TODO: make this more robust?
                /// Make sure the tree always generates unique ids
                entityTree.SetGeneration(id + 1);
            }
            Entity* entity = CreateEntity();

            /// Replace generated id with serialised id
            entities.erase(entity->self->id);
            entity->self->id = id;
            entities[id] = entity->self;

            entity->FromString(itr.second);
            JSON serialisedEntity(itr.second);
            auto parentItr = serialisedEntity.find("Parent");
            int ident = IsInt(parentItr->second) ? ToInt(parentItr->second) : -1;
            if (parentItr != serialisedEntity.end())
            {
                if (ident >= 0)
                {
                    pair<Entity*, int> parentPair(entity, ToInt(parentItr->second));
                    parentMap.push_back(parentPair);
                }
            }
            else
            {
                Logger::EngineLog().Warning("Failed to get entity parent!");
            }
        }
        /// Now setup the entity hierarchy
        for (auto itr : parentMap)
        {
            auto entityItr = entities.find(itr.second);
            if (entityItr != entities.end())
            {
                itr.first->SetParent(entityItr->second->data);
            }
            else
            {
                Logger::EngineLog().Warning("Entity with id '{0}' does not exist in this ECS!", itr.second);
            }
        }
        /// Finally, hook up the serialised pointers
        for (auto itr : serialised_pointers)
        {
            if (IsInt(itr.first))
            {
                auto entityItr = entities.find(ToInt(itr.first));
                if (entityItr != entities.end())
                {
                    for (void** i : itr.second)
                    {
                        *i = ((void*)entityItr->second->data);
                    }
                }
                else
                {
                    Logger::EngineLog().Warning("Could not find entity with id '{0}'.", itr.first);
                }
            }
            else
            {
                string ent_id = SplitLeft(itr.first, ':', "error");
                if (IsInt(ent_id))
                {
                    /// Must be a component pointer
                    auto entityItr = entities.find(ToInt(ent_id));
                    if (entityItr != entities.end())
                    {
                        string comp_type = SplitLeft(SplitRight(itr.first, ':', "error"), ':', "error");
                        ComponentType compTypeId = GetComponentType(comp_type);
                        if (TypeSystem::TypeRegistry<BaseComponent>::IsValidType(compTypeId))
                        {
                            vector<BaseComponent*>& comps = entityItr->second->data->components[compTypeId];
                            string compid = SplitRight(SplitRight(itr.first, ':', "error"), ':', "error");
                            if (IsInt(compid) && !comps.empty())
                            {
                                unsigned int id = ToInt(compid);
                                if (comps.size() >= id)
                                {
                                    for (void** i : itr.second)
                                    {
                                        *i = ((void*)comps[id]);
                                    }
                                }
                                else
                                {
                                    Logger::EngineLog().Warning("Could not find component '{0}'.", itr.first);
                                }
                            }
                            else
                            {
                                Logger::EngineLog().Warning("Could not find component of type \"{0}\" [{1}] with index '{2}'.", comp_type, compTypeId, compid);
                            }
                        }
                        else
                        {
                            Logger::EngineLog().Warning("Could not find component due to invalid type '{0}'.", comp_type);
                        }
                    }
                    else
                    {
                        Logger::EngineLog().Warning("Could not find entity using component id '{0}'.", itr.first);
                    }
                }
                else
                {
                    Logger::EngineLog().Warning("Failed to extract entity id from key '{0}'.", itr.first);
                }
            }
        }
        serialised_pointers.clear();

        for (unsigned int i = 0, counti = TypeSystem::TypeRegistry<BaseComponent>::GetTotalTypes(); i < counti; i++)
        {
            for (auto component : components[i])
            {
                component->OnLoadFinish();
            }
        }

        DEBUG_ASSERT(entities.size() == serialised.size(), "Input entities != created entities!");
    }

    vector<Entity*> EntityComponentSystem::GetRootEntities()
    {
        vector<Entity*> roots;
        for (auto node : entityTree.GetRoots())
        {
            roots.push_back(node->data);
        }
        return roots;
    }

    EntityComponentSystem::~EntityComponentSystem()
    {
        for (Uint32 i = 0, counti = TypeSystem::TypeRegistry<BaseComponent>::GetTotalTypes(); i < counti; i++)
        {
            components[i].clear();
        }
        delete[] components;
        components = nullptr;
    }

}