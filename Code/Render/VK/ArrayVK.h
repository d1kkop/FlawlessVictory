#pragma once
#include "IncVulkan.h"
#include "../../Core/Common.h"

namespace fv
{
    class DeviceVK;

    template <typename T>
    class ArrayVK
    {
    public:
        ArrayVK() = default;
        ~ArrayVK() = default;;

    public:
        void add( const M<T>& vkObject );
        u32 num() const { return (u32)m_VkArray.size(); }
        const typename T::VkType* getAll() const { return m_VkArray.data(); }
        const M<DeviceVK>& device() const { return m_Objects[0]->device(); }

    private:
        List<typename T::VkType> m_VkArray;
        List<M<T>> m_Objects;
    };


    template <typename T>
    void ArrayVK<T>::add( const M<T>& vkObject )
    {
        m_Objects.emplace_back( vkObject );
        m_VkArray.resize( m_Objects.size() );
        for ( u32 i=0; i<(u32)m_Objects.size(); i++ )
        {
            m_VkArray.emplace_back( m_Objects[i]->vk() );
        }
    }

    template <class T> using A = ArrayVK<T>;
}