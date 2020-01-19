#pragma once
#include "IncVulkan.h"
#include "QueueVK.h"
#include "../../Core/Common.h"

namespace fv
{
    class DeviceVK
    {
    public:
        DeviceVK() = default;
        ~DeviceVK();

    public:
        static M<DeviceVK> create( const M<class InstanceVK>& instance,
                                   const Vector<const char*>& requiredExtensions,
                                   const Vector<const char*>& requiredLayers,
                                   bool allowIntegratedGPU,
                                   bool wantGeomtryShader,
                                   bool wantTesselationShader,
                                   u32  numGraphicQueues,
                                   bool wantComputeQueue,
                                   bool wantTransferQueue,
                                   bool wantSparseQueue,
                                   const M<class SurfaceVK>& surface );

        VkPhysicalDevice physical() const { return m_Physical; }
        VkDevice logical() const { return m_Logical; }
        const VkPhysicalDeviceFeatures& features() const { return m_Features; }
        const VkPhysicalDeviceProperties& properties() const { return m_Properties; }
        const VkPhysicalDeviceMemoryProperties& memProperties() const { return m_MemProperties; }

        const M<QueueVK>& graphicsQueue( u32 which=0 ) const { return m_GraphicsQueue[which]; }
        const M<QueueVK>& computeQueue()  const { return m_ComputeQueue; }
        const M<QueueVK>& transferQueue() const { return m_TransferQueue; }
        const M<QueueVK>& sparseQueue()   const { return m_SparseQueue; }
        const M<QueueVK>& presentQueue()  const { return m_PresentQueue; }

        u32 graphicsQueueFamily() const { return m_GraphicsQueueFam; }
        u32 computeQueueFamily()  const { return m_ComputeQueueFam; }
        u32 transferQueueFamily() const { return m_TransferQueueFam; }
        u32 sparseQueueFamily()   const { return m_SparseQueueFam; }
        u32 presentQueueFamily()  const { return m_PresentQueueFam; }

        List<u32> composeQueueFamIndices( bool graphic = true, bool compute = false, bool transfer = false, bool present = false, bool sparse = false );

    private:
        VkPhysicalDevice m_Physical {};
        VkDevice m_Logical {};
        VkPhysicalDeviceFeatures m_Features {};
        VkPhysicalDeviceProperties m_Properties {};
        VkPhysicalDeviceMemoryProperties m_MemProperties {};
        List<M<QueueVK>> m_GraphicsQueue;
        M<QueueVK> m_ComputeQueue {};
        M<QueueVK> m_TransferQueue {};
        M<QueueVK> m_SparseQueue {};
        M<QueueVK> m_PresentQueue {};
        u32 m_GraphicsQueueFam = -1;
        u32 m_ComputeQueueFam  = -1;
        u32 m_TransferQueueFam = -1;
        u32 m_SparseQueueFam   = -1;
        u32 m_PresentQueueFam  = -1;
    };
}