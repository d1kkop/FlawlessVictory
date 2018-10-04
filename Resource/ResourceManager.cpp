#include "ResourceManager.h"
#include "Resource.h"
#include "../Core/LogManager.h"
#include "../Core/JobManager.h"

namespace fv
{
    Resource* ResourceManager::load(u32 type, const String& name)
    {
        const TypeInfo* ti = typeManager()->typeInfo(type);
        if (!ti) return nullptr;
        String ext = ti->extension;
        if (  ext.size() < 2 || ext[0] != '.'  )
        {
            LOGW("Type %s has invalid extension %s.", ti->name->c_str(), ext);
        }
        Path path = name;
        path.replace_extension( ext );
        M<Resource> resource = M<Resource>( (Resource*)ti->createFunc(1) );
        jobManager()->addJob([=]()
        {
            resource->load(path);
        }, [=](Job* j) 
        {
            resource->onDoneOrCancelled(j);
        });
        return nullptr;
    }
}