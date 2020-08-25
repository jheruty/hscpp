#pragma once

#include <windows.h>
#include <string>
#include <sstream>
#include <filesystem>

#define HSCPP_LOG_PREFIX __func__ << "[" << __LINE__ << "]: "

namespace hscpp
{

    enum class LogLevel
    {
        Debug,
        Info,
        Warning,
        Error,
    };

    class EndLog
    {
    public:
        EndLog() {};
        EndLog(const std::string& value);

        std::string Value() const;

    private:
        std::string m_Value;
    };

    class LastErrorLog
    {};

    class ErrorLog
    {
    public:
        ErrorLog(DWORD value);
        ErrorLog(const std::error_code& value);

        DWORD Value() const;

    private:
        DWORD m_Value;
    };

    class LoggerStream
    {
    public:
        LoggerStream(bool bEnabled);
        LoggerStream& operator<<(const std::string& str);

        LoggerStream& operator<<(const LastErrorLog& lastErrorLog);
        LoggerStream& operator<<(const ErrorLog& errorLog);
        void operator<<(const EndLog& endLog);

        template <typename T>
        LoggerStream& operator<<(const T& val)
        {
            if (!m_bEnabled)
            {
                return *this;
            }

            m_Stream << val;
            return *this;
        }

    private:
        bool m_bEnabled = true;
        std::wstringstream m_Stream;
    };

    class Log
    {
    public:
        static void SetLogLevel(LogLevel level);

        static void EnableBuildLogging();
        static void DisableBuildLogging();

        static LoggerStream Debug();
        static LoggerStream Info();
        static LoggerStream Warning();
        static LoggerStream Error();

        static LoggerStream Build();

    private:
        static bool s_bLogBuild;
        static LogLevel s_LogLevel;

        static bool IsLogLevelActive(LogLevel level);
    };
}

