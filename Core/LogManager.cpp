#include "LogManager.h"
#include "Functions.h"
#include <cstdlib>
#include <cstdarg>

namespace fv
{
    void LogManager::log( LogType type, const char* functionName, u64 line, const char* msg, ... )
    {
        rscoped_lock lk(m_LogMutex);

        char buff[8192];
        va_list myargs;
        va_start(myargs, msg);
        vsprintf_s(buff, 8190, msg, myargs);
        va_end(myargs);

        String finalStr = localTime();
        if ( type != LogType::Message )
        {
            String warnStr;
            switch ( type )
            {
            case LogType::Warning:
                warnStr = " WARNING: ";
                break;
            case LogType::Error:
                warnStr = " ERROR ";
                break;
            }
            finalStr += warnStr;
        }

        if ( m_LogLineAndFile ) 
        {
            finalStr +=  String(functionName) + " line: " + std::to_string(line);
        }

        finalStr += " " + String(buff) + "\n";

        // On first open attach new session in front.
        static bool isFirstOpen = true;
        if ( isFirstOpen )
        {
            isFirstOpen = false;
            log(LogType::Message, FV_FUNCTION, FV_LINE, "------- New Session --------");
            log(LogType::Message, FV_FUNCTION, FV_LINE, "----------------------------");
        }

        logToFile(finalStr.c_str());
        logToIde(finalStr.c_str());
    }

    void LogManager::setOptions(bool logFile, bool logIde, bool logFileAndLine)
    {
        m_LogToFile = logFile;
        m_LogToIde = logIde;
        m_LogLineAndFile = logFileAndLine;
    }

    void LogManager::logToFile(const char* msg)
    {
        if ( !m_LogToFile ) return;

        static const char* fileName = format("fv_log_%s.txt", localTime()).c_str();
        
        // (Re)open file
        FILE* f;
    #if FV_SECURE_CRT
        fopen_s(&f, fileName, "a");
    #else
        f = fopen(fileName, "a");
    #endif

        if ( !f ) return;

        fprintf(f, msg);
        fclose(f);
    }

    void LogManager::logToIde(const char* msg)
    {
        if ( !m_LogToIde ) return;
    #if FV_INCLUDE_WINHDR
        ::OutputDebugString(msg);
    #endif
    }
}