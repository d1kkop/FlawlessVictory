#pragma once
#include "GraphicResource.h"
#if FV_VULKAN
#include <vulkan/vulkan.h>

namespace fv
{
    class GraphicResourceVK: public GraphicResource
    {
    public:
        virtual ~GraphicResourceVK() override;

    public:
        void init(GraphicType type) override;
        void freeResource();
        bool updateImage(u32 width, u32 height, const byte* data, u32 size, ImageFormat format) override;
        bool updateBuffer(const byte* data, u32 size, BufferFormat format) override;
        bool updateShaderCode(Vector<char>& code) override;
        bool updateMeshData(const Submesh& submesh) override;

        VkDevice m_Device{};
        union
        {
            VkImage m_Image;
            VkBuffer m_Buffer;
            VkShaderModule m_Shader;
            VkBuffer m_Indices;
        };
        Vector<VkBuffer> m_Vertices;

        friend class RenderManagerVK;
    };
}
#endif