#pragma once
#include "PCH.h"
#include "Common.h"

#define LOG( msg, ... )  logManager()->log( LogType::Message, FV_FUNCTION, FV_LINE, msg, __VA_ARGS__ )
#define LOGW( msg, ... ) logManager()->log( LogType::Warning, FV_FUNCTION, FV_LINE, msg, __VA_ARGS__ )
#define LOGC( msg, ... ) logManager()->log( LogType::Error, FV_FUNCTION, FV_LINE, msg, __VA_ARGS__ )

namespace fv
{
    enum LogType
    {
        Message,
        Warning,
        Error
    };

    class LogManager
    {
    public:
        FV_DLL LogManager();
        FV_TS FV_DLL void log( LogType type, const char* functionName, u64 line, const char* msg, ... );
        FV_DLL void setOptions( bool logFile, bool logIde );

    private:
        void logToFile(const char* msg);
        void logToIde(const char* msg);

        RMutex m_LogMutex;
        bool m_LogToFile = true;
        bool m_LogToIde  = true;
        String m_Filename;
    };

    FV_DLL class LogManager* logManager();
    FV_DLL void deleteLogManager();
}