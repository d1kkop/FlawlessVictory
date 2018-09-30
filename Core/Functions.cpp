#include "Functions.h"
#include <ctime>
#include <cstdlib>
#include <cstdarg>
#include <string>
#include <chrono>
using namespace std;
using namespace chrono;

namespace fv
{
    double s_StartTime = epochTime();

    String localTime()
    {
        static Mutex timeMutex;
        scoped_lock lk(timeMutex);
        time_t rawtime;
        struct tm timeinfo;
        ::time (&rawtime);
        localtime_s(&timeinfo, &rawtime);
        char asciitime[256];
        asctime_s(asciitime, 256, &timeinfo);
        // For some reason it attaches a new line..
        char* p = strstr(asciitime, "\n");
        if ( p )
        {
            *p ='\0';
            return asciitime;
        }
        return "";
    }

    String format(const char* fmt, ...)
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

    double epochTime()
    {
        return duration_cast<duration<double>>(high_resolution_clock::now().time_since_epoch()).count();
    }

    float time()
    {
        return (float)(epochTime()-s_StartTime);
    }

}