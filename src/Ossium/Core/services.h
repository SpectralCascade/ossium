#ifndef SERVICES_H
#define SERVICES_H

#include "logging.h"
#include "typefactory.h"

namespace Ossium
{

    namespace Internal
    {
        class OSSIUM_EDL ServiceBase
        {
        public:
            /// Called before main logic execution.
            virtual void PreUpdate()
            {
            }

            /// Called after main logic execution, but before rendering.
            virtual void PostUpdate()
            {
            }

            /// Called after rendering.
            virtual void PostRender()
            {
            }

        };
    }

    /// Non-static services locator.
    class OSSIUM_EDL ServicesProvider
    {
    public:

        /// Specify what services are provided in the constructor arguments.
        template<typename ...Args>
        ServicesProvider(Args&& ...args)
        {
            /// Instantiate services by type
            const Uint32 totalTypes = TypeSystem::TypeRegistry<Internal::ServiceBase>::GetTotalTypes();
            services = new Internal::ServiceBase*[totalTypes];
            for (unsigned int i = 0; i < totalTypes; i++)
            {
                services[i] = nullptr;
            }
            /// Now hook up the individual services
            SetService(forward<Args>(args)...);
        }

        virtual ~ServicesProvider();

        /// Attempts to return a pointer to a service matching the specified type.
        /// Returns nullptr if a service instance of the specified type does not exist.
        template<typename T>
        T* GetService()
        {
            Uint32 index = T::__services_entry.GetType();
            if (index < TypeSystem::TypeRegistry<Internal::ServiceBase>::GetTotalTypes())
            {
                return reinterpret_cast<T*>(services[index]);
            }
            Logger::EngineLog().Warning("Failed to get service, invalid service type requested.");
            return nullptr;
        }

        /// Call this just before the main logic update.
        void PreUpdate()
        {
            for (unsigned int i = 0, counti = TypeSystem::TypeRegistry<Internal::ServiceBase>::GetTotalTypes(); i < counti; i++)
            {
                if (services[i] != nullptr)
                {
                    services[i]->PreUpdate();
                }
            }
        }

        /// Call this just after the main logic update.
        void PostUpdate()
        {
            for (unsigned int i = 0, counti = TypeSystem::TypeRegistry<Internal::ServiceBase>::GetTotalTypes(); i < counti; i++)
            {
                if (services[i] != nullptr)
                {
                    services[i]->PostUpdate();
                }
            }
        }

        /// Call this at the end of a frame after rendering.
        void PostRender()
        {
            for (unsigned int i = 0, counti = TypeSystem::TypeRegistry<Internal::ServiceBase>::GetTotalTypes(); i < counti; i++)
            {
                if (services[i] != nullptr)
                {
                    services[i]->PostRender();
                }
            }
        }

    protected:
        /// Services by type
        Internal::ServiceBase** services = nullptr;

    private:
        /// Base case
        void SetService();

        /// Recursive step through template parameter pack and set up the services.
        template<typename T, typename ...Args>
        void SetService(T service, Args&& ...args)
        {
            /// Recursive step first so services are set in reverse.
            /// This means if the same service type is provided more than once,
            /// the first instance of that type is used and the other instances are discarded.
            SetService(forward<Args>(args)...);
            services[remove_pointer<T>::type::__services_entry.GetType()] = service;
        }

    };

    /// Common engine services (e.g. Renderer, ResourceController) should inherit from this CRTP mix-in
    template<class Derived>
    class OSSIUM_EDL Service : public Internal::ServiceBase
    {
    public:
        friend class ServicesProvider;

    protected:
        static TypeSystem::TypeRegistry<Internal::ServiceBase> __services_entry;

    };

    template<class Derived>
    TypeSystem::TypeRegistry<Internal::ServiceBase> Service<Derived>::__services_entry;

}

#endif // SERVICES_H