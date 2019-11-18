#include "component.h"

namespace Ossium
{

    REGISTER_ABSTRACT_COMPONENT(Component);

    ///
    /// Component
    ///

    Point& Component::WorldPosition()
    {
        return entity->AddComponentOnce<Transform2>()->position();
    }

    Rotation& Component::WorldRotation()
    {
        return entity->AddComponentOnce<Transform2>()->rotation();
    }

    Transform& Component::WorldTransform()
    {
        return entity->AddComponentOnce<Transform2>()->transform();
    }

    ///
    /// GraphicComponent
    ///

    void GraphicComponent::SetRenderLayer(int layer)
    {
        if (rendererInstance != nullptr)
        {
            rendererInstance->Unregister(this, renderLayer);
            renderLayer = rendererInstance->Register(this, layer);
        }
    }

    int GraphicComponent::GetRenderLayer()
    {
        return renderLayer;
    }

    GraphicComponent::GraphicComponent()
    {
    }

    GraphicComponent::~GraphicComponent()
    {
    }

    void GraphicComponent::OnCreate()
    {
        OnInit(GetService<Renderer>(), -1);
    }

    void GraphicComponent::OnDestroy()
    {
        OnRemoveGraphics();
    }

    void GraphicComponent::OnClone(BaseComponent* src)
    {
    }

    void GraphicComponent::Update()
    {
    }

    void GraphicComponent::OnInit(Renderer* renderer, int layer)
    {
        renderLayer = layer >= 0 ? layer : renderLayer;
        rendererInstance = renderer != nullptr ? renderer : rendererInstance;
        if (rendererInstance != nullptr)
        {
            renderLayer = rendererInstance->Register(this, renderLayer);
        }
    }

    void GraphicComponent::OnRemoveGraphics()
    {
        if (rendererInstance != nullptr)
        {
            rendererInstance->Unregister(this, renderLayer);
        }
    }

}