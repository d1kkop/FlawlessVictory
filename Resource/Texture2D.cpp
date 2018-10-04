#include "Texture2D.h"
#include "../Core/JobManager.h"
#include "../Core/Directories.h"
#include "../Render/RenderManager.h"

namespace fv
{
    void Texture2D::load(const Path& path)
    {
    #if FV_STB
        stbi_uc* pixels = stbi_load(path.string().c_str(), (int*)&m_Width, (int*)&m_Height, (int*)&m_Channels, STBI_rgb_alpha);
        if (!pixels) 
        {
            LOGW("Failed to load %s.", path.c_str());
            return;
        }
        m_Graphic = renderManager()->createGraphic<Texture2D>();
     //   m_Graphic->updateImage2D( m_Width, m_Height, pixels, m_Channels );
        stbi_image_free( pixels );
    #endif
    }

    void Texture2D::onDoneOrCancelled(class Job* j)
    {
        j->waitAndFree();
    }

}