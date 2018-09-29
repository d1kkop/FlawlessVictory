#pragma once
#include "Common.h"

#define LOG( msg, ... )  logManager()->log( LogType::Message, FV_FUNCTION, FV_LINE, msg, __VA_ARGS__ )
#define LOGW( msg, ... ) logManager()->log( LogType::Warning, FV_FUNCTION, FV_LINE, msg, __VA_ARGS__ )
#define LOGC( msg, ... ) logManager()->log( LogType::Critical, FV_FUNCTION, FV_LINE, msg, __VA_ARGS__ )

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
        // Thread safe
        FV_DLL void log( LogType type, const char* functionName, u64 line, const char* msg, ... );
        FV_DLL void setOptions( bool logFile, bool logIde, bool logFileAndLine )
        {
            m_LogToFile = logFile;
            m_LogToIde = logIde;
            m_LogLineAndFile = logFileAndLine;
        }

    private:
        void logToFile(const char* msg);
        void logToIde(const char* msg);

        RMutex m_LogMutex;
        bool m_LogToFile = true;
        bool m_LogToIde  = true;
        bool m_LogLineAndFile = true;
    };
}