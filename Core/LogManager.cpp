#include "PCH.h"
#include "LogManager.h"
#include "Functions.h"
#include "Directories.h"
#include <cstdlib>
#include <cstdarg>

namespace fv
{

    LogManager::LogManager()
    {
        Path filename = Format( "FVLOG_%ld.txt", (u64)(EpochTime()*1000) );
        m_Filename = ( Directories::log() / filename ).string();
    }

    void LogManager::log(LogType type, const char* functionName, u64 line, const char* msg, ...)
    {
        rscoped_lock lk(m_LogMutex);

        char buff[8192];
        va_list myargs;
        va_start(myargs, msg);
        vsprintf_s(buff, 8190, msg, myargs);
        va_end(myargs);

        String finalStr; // Do not add time to every line to avoid verbosity = LocalTime();
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

        // (Re)open file
        FILE* f;
    #if FV_SECURE_CRT
        fopen_s(&f, m_Filename.c_str(), "a");
    #else
        f = fopen(m_Filename.c_str(), "a");
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

    LogManager* g_LogManager {};
    LogManager* logManager() { return CreateOnce(g_LogManager); }
    void deleteLogManager() { delete g_LogManager; g_LogManager=nullptr; }
}