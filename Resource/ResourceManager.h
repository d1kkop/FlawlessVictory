#pragma once
#include "../Core/Common.h"

namespace fv
{
    class Resource;

    class ResourceManager
    {
    public:
        FV_TS Resource* load(u32 type, const String& name);
        template <class T> FV_TS T* load();

    private:
        // Note resources are not recycled but shared, so to have a shared ptr.
        Map<String, M<Resource>> m_NameToResource;
        Map<u32, Vector<M<Resource>>> m_TypeToResource;
    };


    template <class T>
    T* fv::ResourceManager::load()
    {
        return load(T::type());
    }

}
