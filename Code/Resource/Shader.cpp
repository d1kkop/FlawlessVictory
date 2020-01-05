#include "Shader.h"
#include "Assets.h"
#include "ResourceManager.h"
#include "ShaderCompiler.h"
#include "../Core/Functions.h"
#include "../Core/Directories.h"
#include "../Core/LogManager.h"
#include "../Render/RenderManager.h"
#include "../Resource/PatchManager.h"

namespace fv
{
    FV_TYPE_IMPL(Shader)

    Shader::~Shader()
    {
    }

    void Shader::setDeviceShader(const M<DeviceResource>& deviceShader)
    {
        FV_CHECK_MO();
        m_DeviceShader = deviceShader;
    }

    void Shader::load_RT(const ResourceToLoad& rtl)
    {
        Vector<char> code;

        // Check if compiled file is already there
        Path binPath = Directories::intermediateShaders() / rtl.loadPath.filename();
        binPath.replace_extension( Assets::shaderBinExtension() );
        if ( rtl.reimport || !LoadBinaryFile(binPath.string().c_str(), code) )
        {
            code.clear(); // In case binary load partially succeeded
            if ( !shaderCompiler()->compileShader(rtl.loadPath, code) )
            {
                LOGW("Failed to load shader %s.", rtl.loadPath.string().c_str());
                return;
            }
        }

        if ( !code.empty() )
        {
            M<DeviceResource> graphic = renderManager()->createShader( code.data(), (u32)code.size() );
            Patch* p = patchManager()->createPatch(PatchType::ShaderCode);
            p->graphic  = graphic;
            p->resource = rtl.resource;
            p->submit();
            
        }
    }
}