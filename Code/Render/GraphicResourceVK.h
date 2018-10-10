#pragma once
#include "GraphicResource.h"
#if FV_VULKAN
#include <vulkan/vulkan.h>

namespace fv
{
    class GraphicResourceVK: public GraphicResource
    {
        void freeResource() override;
        bool updateImage(u32 width, u32 height, const byte* data, u32 size, ImageFormat format) override;
        bool updateBuffer(const byte* data, u32 size, BufferFormat format) override;
        bool updateShaderCode(Vector<char>& code) override;

    private:
        VkDevice m_Device{};
        union
        {
            VkImage m_Image;
            VkBuffer m_Buffer;
            VkShaderModule m_Shader;
        };

        friend class RenderManagerVK;
    };
}
#endif