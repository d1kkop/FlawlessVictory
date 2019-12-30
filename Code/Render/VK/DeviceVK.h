#pragma once
#include "IncVulkan.h"
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
                                   bool wantPresentSupport );

        VkPhysicalDevice physical() const { return m_Physical; }
        VkDevice logical() const { return m_Logical; }
        const VkPhysicalDeviceFeatures& features() const { return m_Features; }
        const VkPhysicalDeviceProperties& properties() const { return m_Properties; }
        const VkPhysicalDeviceMemoryProperties& memProperties() const { return m_MemProperties; }

        VkQueue graphicsQueue( u32 which=0 ) { return m_GraphicsQueue[which]; }
        VkQueue computeQueue()  const { return m_ComputeQueue; }
        VkQueue transferQueue() const { return m_TransferQueue; }
        VkQueue sparseQueue()   const { return m_SparseQueue; }
        VkQueue presentQueue()  const { return m_PresentQueue; }

    private:
        VkPhysicalDevice m_Physical {};
        VkDevice m_Logical {};
        VkPhysicalDeviceFeatures m_Features {};
        VkPhysicalDeviceProperties m_Properties {};
        VkPhysicalDeviceMemoryProperties m_MemProperties {};
        List<VkQueue> m_GraphicsQueue;
        VkQueue m_ComputeQueue {};
        VkQueue m_TransferQueue {};
        VkQueue m_SparseQueue {};
        VkQueue m_PresentQueue {};
    };
}