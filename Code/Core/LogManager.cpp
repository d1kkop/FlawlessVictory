#include "PCH.h"
#include "LogManager.h"
#include "Functions.h"
#include "Directories.h"
#include "IncWindows.h"
using namespace std;

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

        String finalStr = "-> "; // Do not add time to every line to avoid verbosity = LocalTime();
        bool logLineAndFile = false;
        if ( type != LogType::Message )
        {
            String warnStr;
            switch ( type )
            {
            case LogType::Warning:
                warnStr = "WARNING: ";
                logLineAndFile = true;
                break;
            case LogType::Error:
                warnStr = "ERROR ";
                logLineAndFile = true;
                break;
            }
            finalStr += warnStr;
        }

        if ( logLineAndFile ) 
        {
            finalStr += String(buff) + " In: " + String(functionName) + " line: " + std::to_string(line) + "\n";
        }
        else
        {
            finalStr += String(buff) + "\n";
        }


        logToFile(finalStr.c_str());
        logToIde(finalStr.c_str());
    }

    void LogManager::setOptions(bool logFile, bool logIde)
    {
        m_LogToFile = logFile;
        m_LogToIde = logIde;
    }

    void LogManager::logToFile(const char* msg)
    {
        if ( !m_LogToFile ) return;

        // (Re)open file
        ofstream file( m_Filename.c_str(), ios::app );
        if ( file.is_open() )
        {
            try
            {
                file << msg;
                file.close();
            }
            catch (...)
            {
                if ( file.is_open() )
                    file.close();
            }
        }
    }

    void LogManager::logToIde(const char* msg)
    {
        if ( !m_LogToIde ) return;
    #if (FV_INCLUDE_WINHDR && _MSC_VER)
        ::OutputDebugString(msg);
    #endif
    }

    LogManager* g_LogManager {};
    LogManager* logManager() { return CreateOnce(g_LogManager); }
    void deleteLogManager() { delete g_LogManager; g_LogManager=nullptr; }
}