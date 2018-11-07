#pragma once
#include "../Core/Component.h"

namespace fv
{
    class RenderComponent: public Component
    {
    public:
        FV_MO FV_DLL RenderComponent();
        FV_MO FV_DLL ~RenderComponent() override;

        virtual void cullMT(u32 tIdx) { }
        virtual void drawMT(u32 tIdx) { }

        FV_DLL void castShadows( bool cast ) { m_CastShadows = cast; }
        FV_DLL void receiveShadows( bool receive ) { m_ReceiveShadows = receive; }
        
        FV_DLL u32 deviceIdx() const { return m_DeviceIdx; }
        FV_DLL bool culled() const { return m_Culled; }
        FV_DLL bool castShadows() const { return m_CastShadows; }
        FV_DLL bool receiveShadows() const { return m_ReceiveShadows; }

    protected:
        u32 m_DeviceIdx;
        bool m_Culled = false;
        bool m_CastShadows = true;
        bool m_ReceiveShadows = true;
    };

}