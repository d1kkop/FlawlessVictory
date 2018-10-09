#pragma once
#include "UnitTest.h"
#include "../Resources.h"
#include "../Resource/ResourceManager.h"
#include <cassert>
using namespace fv;


UTESTBEGIN(LoadTexture2D)
{
    Vector<M<Texture2D>> textures;
    printf("10 Times loop over scattered and contiguous memory\n");
    for ( u32 t = 0; t < 10; t ++ )
    {
        u32 tk = 1000;
        for ( u32 u =0; u < tk; u++ )
        {
            M<Texture2D> tex2D = resourceManager()->load<Texture2D>("pig.jpg");
            textures.emplace_back( tex2D );
        }
    }
    textures.clear();
    resourceManager()->cleanupResourcesWithoutReferences();
    return true;
}
UNITTESTEND(LoadTexture2D)