#pragma once
#include "../Core/Common.h"
#include "../Core/Reflection.h"

namespace fv
{
    class GraphicResource;

    struct RenderSetup
    {
        bool createMainWindow;
        bool creatSecondaryWindow;
        String mainWindowName;
        String secondaryWindowName;
        u32 mainWindowWidth, mainWindowHeight;
        u32 secondaryWindowWidth, secondaryWindowHeight;
        bool mainWindowFullscreen;
        bool secondaryWindowFullscreen;
    };

    class RenderManager
    {
    public:
        virtual ~RenderManager() = default;
        virtual bool initGraphics() = 0;
        virtual void closeGraphics() = 0;
        virtual GraphicResource* createGraphic() = 0; // NOT THREAD SAFE TODO (Called from Resource::load)
        virtual void render(const class Camera* camera) = 0;

        FV_DLL void freeGraphic(GraphicResource* graphic);

        template <class T> GraphicResource* createGraphic();

    };


    template <class T> 
    GraphicResource* RenderManager::createGraphic()
    {
        GraphicResource* graphic = createGraphic();
        graphic->m_ResourceType = T::type();
        return graphic;
    }

    FV_DLL RenderManager* renderManager();
    FV_DLL void deleteRenderManager();
}