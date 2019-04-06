#ifndef _CONSOLE_H__
#define _CONSOLE_H__

#include "standard/events.h"
#include "standard/Platform/Define.h"
#include <mutex>

namespace System
{
    enum LogType
    {
        LogNormal = 0,
        LogDetails,
        LogDebug,
        LogError,
        LogWarning,
        LogDBError,
        LogMax
    };

    enum LogLevel : int8
    {
        LOG_LVL_NONE = -1,
        LOG_LVL_MINIMAL = 0,                                    // unconditional and errors
        LOG_LVL_ERROR,
        LOG_LVL_BASIC,
        LOG_LVL_DETAIL,
        LOG_LVL_DEBUG,
        LOG_LVL_MAX
    };

    enum Color
    {
        BLACK,
        RED,
        GREEN,
        BROWN,
        BLUE,
        MAGENTA,
        CYAN,
        GREY,
        YELLOW,
        LRED,
        LGREEN,
        LBLUE,
        LMAGENTA,
        LCYAN,
        WHITE
    };

    const int Color_count = int(WHITE) + 1;

    struct ConsoleLogEventArg : public ::System::Events::EventArg
    {
        ConsoleLogEventArg() : i_message(NULL), i_logType(-1)
        {
        }

        explicit ConsoleLogEventArg(std::string* message, short logType) : i_message(message), i_logType(logType)
        {
        }
        virtual ~ConsoleLogEventArg()
        {
        }

        std::string* i_message;
        short i_logType;
    };

    class TC_CFRAMEWORK_API Log
    {
    public:
        FILE* openLogFile(char const *configFileName, char const *configTimeStampFlag, char const *mode);
        FILE* logfile;
        FILE* dberLogfile;
        uint8_t m_logLevel;

        static inline LogLevel LogTypeToLevel(LogType type)
        {
            LogLevel level = LogLevel::LOG_LVL_NONE;
            switch (type)
            {
                case LogNormal:  level = LOG_LVL_MINIMAL; break;
                case LogDBError:
                case LogWarning:
                case LogError:  level = LOG_LVL_ERROR; break;
                case LogDetails: level = LOG_LVL_BASIC; break;
                case LogDebug: level = LOG_LVL_DEBUG; break;
                default:
                    break;
            }

            return level;
        }

        bool HasLogLevelOrHigher(LogLevel loglvl) const { return m_logLevel >= loglvl; }

    public:
        bool m_colored;
        bool m_includeTime;

        std::string m_logsDir;
        std::string m_logsTimestamp;
        Color m_colors[LogType::LogMax];

        ::System::Events::EventHandlerThreadSafe< ConsoleLogEventArg, std::mutex > OnConsoleWrite;

        static Log* Instance();

        Log();
        ~Log();

        void Initialize();
        void InitColors(const std::string& init_str);

        void SetLogLevel(uint8_t Level) { m_logLevel = Level; }
        uint8_t GetLogLevel() const { return m_logLevel; }
        void SetColor(bool stdout_stream, Color color, std::string& unix_result);
        void ResetColor(bool stdout_stream, std::string& unix_result);
        void outTime(std::string& result);
        static void outTimestamp(std::ostringstream& outFileStream);
        static std::string GetTimestampStr();

        void WriteToConsole(const char* fmt, LogType logType, bool is_inline = false);
        void WriteLine(FORMAT_STRING(const char* fmt), ...)           ATTR_PRINTF(2, 3);
        void WriteErrorLine(FORMAT_STRING(const char* fmt), ...)      ATTR_PRINTF(2, 3);
        void WriteErrorDbLine(FORMAT_STRING(const char* fmt), ...)    ATTR_PRINTF(2, 3);
        void WriteDebugLine(FORMAT_STRING(const char* fmt), ...)      ATTR_PRINTF(2, 3);
        void WriteDetailLine(FORMAT_STRING(const char* fmt), ...)     ATTR_PRINTF(2, 3);
        void WriteWarnLine(FORMAT_STRING(const char* fmt), ...)       ATTR_PRINTF(2, 3);

        // (slow but safe)
        template<typename Format, typename... Args>
        inline void FilterMessage(const char* filter, LogType const type, Format&& fmt, Args&&... args)
        {
            LogLevel level = LogTypeToLevel(type);

            if (!HasLogLevelOrHigher(level))
                return;

            char buf[16000];
            std::sprintf(buf, std::forward<Format>(fmt), std::forward<Args>(args)...);

            WriteToConsole(buf, type);
        }
    };

    inline Log* Console()
    {
        return Log::Instance();
    }
}

#define LOG_EXCEPTION_FREE(filterType__, level__, ...) \
    do { \
        try \
        { \
            ::System::Log::Instance()->FilterMessage(filterType__, level__, __VA_ARGS__ ); \
        } \
        catch (std::exception& e) \
        { \
            ::System::Log::Instance()->FilterMessage("server", ::System::LogType::LogError, "Wrong format occurred (%s) at %s:%u.", \
                e.what(), __FILE__, __LINE__); \
        } \
    } while(0)

#define TC_LOG_FATAL_ERROR(filterType__, ...) LOG_EXCEPTION_FREE(filterType__, ::System::LogType::LogError,  __VA_ARGS__); assert(false)
#define TC_LOG_ERROR(filterType__, ...) LOG_EXCEPTION_FREE(filterType__, ::System::LogType::LogError,  __VA_ARGS__)
#define TC_LOG_TRACE(filterType__, ...) LOG_EXCEPTION_FREE(filterType__, ::System::LogType::LogDetails,  __VA_ARGS__)
#define TC_LOG_INFO(filterType__, ...) LOG_EXCEPTION_FREE(filterType__, ::System::LogType::LogNormal,  __VA_ARGS__)
#define TC_LOG_WARN(filterType__, ...) LOG_EXCEPTION_FREE(filterType__, ::System::LogType::LogWarning, __VA_ARGS__)
#define TC_LOG_DEBUG(filterType__, ...) LOG_EXCEPTION_FREE(filterType__, ::System::LogType::LogDebug,  __VA_ARGS__)
#define TC_LOG(filterType__, ...) LOG_EXCEPTION_FREE(filterType__, ::System::LogType::LogNormal,  __VA_ARGS__)

#define outstring_log(...) LOG_EXCEPTION_FREE("unused", ::System::LogType::LogNormal,  __VA_ARGS__)
#define detail_log(...)    LOG_EXCEPTION_FREE("unused", ::System::LogType::LogDetails, __VA_ARGS__)
#define debug_log(...)     LOG_EXCEPTION_FREE("unused", ::System::LogType::LogDebug,   __VA_ARGS__)
#define error_log(...)     LOG_EXCEPTION_FREE("unused", ::System::LogType::LogError,   __VA_ARGS__)
#define error_db_log(...)  LOG_EXCEPTION_FREE("unused", ::System::LogType::LogDBError, __VA_ARGS__)

#endif //_CONSOLE_H__