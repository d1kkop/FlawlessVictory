#include "PCH.h"
#include "Functions.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "IncWindows.h"
#include "IncSDL.h"
#ifndef WIN32
    #include <unistd.h>
#endif
#ifdef WIN32
#define stat _stat
#endif
using namespace std;
using namespace chrono;

namespace fv
{
    double g_StartTime = EpochTime();
    bool g_EngineClosing = false;

    String LocalTime()
    {
        static Mutex timeMutex;
        scoped_lock lk(timeMutex);
        time_t rawtime;
        ::time (&rawtime);
        char asciitime[256];
    #if FV_SECURE_CRT
        struct tm timeinfo;
        localtime_s(&timeinfo, &rawtime);
        asctime_s(asciitime, 256, &timeinfo);
    #else
        auto* tm = localtime(&rawtime);
        strcpy(asciitime, asctime(tm));
    #endif
        // For some reason it attaches a new line..
        char* p = strstr(asciitime, "\n");
        if ( p )
        {
            *p ='\0';
            return asciitime;
        }
        return "";
    }

    String Format(const char* fmt, ...)
    {
        char buff[8192];
        va_list myargs;
        va_start(myargs, fmt);
    #if FV_SECURE_CRT
        ::vsprintf_s(buff, 8190, fmt, myargs);
    #else
        ::vsprintf(buff, fmt, myargs);
    #endif
        va_end(myargs);
        return buff;
    }

    double EpochTime()
    {
        return duration_cast<duration<double>>(high_resolution_clock::now().time_since_epoch()).count();
    }

    float RunTime()
    {
        return (float)(EpochTime()-g_StartTime);
    }

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

    u32 Random()
    {
        // TODO change for better random
        return ::rand();
    }

    void Suspend(double seconds)
    {
        std::this_thread::sleep_for( duration<double>(seconds) );
    }

    void StringCopy(char* dst, u32 dstSize, const char* src)
    {
    #if FV_SECURE_CRT
        strcpy_s( dst, dstSize, src );
    #else
        strcpy( dst,src );
    #endif
    }

    const char* NewString(const char* src, u32 len)
    {
        if ( len == -1 )
            len = (u32) strlen(src);
        char* c = new char[len+1];
        StringCopy(c, len+1, src);
        return c;
    }

    bool LoadBinaryFile(const char* path, Vector<char>& data)
    {
        ifstream file(path, std::ios::ate | std::ios::binary);
        if ( file.is_open() )
        {
            try 
            {
                u64 pos = file.tellg();
                data.resize(pos);
                file.seekg(0);
                file.read(data.data(), data.size());
                file.close();
            }
            catch ( ... )
            {
                if ( file.is_open() ) file.close();
                return false;
            }
        }
        else
        {
            return false;
        }
        return true;
    }

    bool OpenFile(FILE*& file, const char* path, const char* mode)
    {
    #if FV_SECURE_CRT
        fopen_s( &file, path, mode );
    #else
        f = fopen(path, mode);
    #endif
        return file;
    }

    void CloseFile(FILE* file, bool flush)
    {
        if (!file) return;
        if ( flush ) fflush(file);
        fclose(file);
    }

    u64 FileModifiedTime(const char* path)
    {
        struct stat st;
        if ( stat(path, &st) == 0 )
        {
            return (u64)st.st_mtime;
        }
        return -1;
    }

    bool IsEngineClosing()
    {
        return g_EngineClosing;
    }

    void SetEngineClosing()
    {
        g_EngineClosing = true;
    }

}