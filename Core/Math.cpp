#include "Math.h"
#include <ctime>

namespace fv
{
    u32 Hash32(const char* key, u32 len)
    {
        uint32_t seed = 88123;
        if ( len > 3 )
        {
            const uint32_t* key_x4 = (const uint32_t*)key;
            size_t i = len >> 2;
            do
            {
                uint32_t k = *key_x4++;
                k *= 0xcc9e2d51;
                k = (k << 15) | (k >> 17);
                k *= 0x1b873593;
                seed ^= k;
                seed = (seed << 13) | (seed >> 19);
                seed = (seed * 5) + 0xe6546b64;
            }
            while ( --i );
            key = (const char*)key_x4;
        }
        if ( len & 3 )
        {
            size_t i = len & 3;
            uint32_t k = 0;
            key = &key[i - 1];
            do
            {
                k <<= 8;
                k |= *key--;
            }
            while ( --i );
            k *= 0xcc9e2d51;
            k = (k << 15) | (k >> 17);
            k *= 0x1b873593;
            seed ^= k;
        }
        seed ^= len;
        seed ^= seed >> 16;
        seed *= 0x85ebca6b;
        seed ^= seed >> 13;
        seed *= 0xc2b2ae35;
        seed ^= seed >> 16;
        return seed;
    }

}