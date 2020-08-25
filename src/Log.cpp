#include <iostream>

#include "hscpp/Log.h"
#include "hscpp/Util.h"

#pragma warning(disable:4996) // Disable security warning to allow std::mbstowcs.

namespace hscpp
{
    LogLevel Log::s_LogLevel = LogLevel::Info;
    bool Log::s_bLogBuild = true;

    EndLog::EndLog(const std::string& value)
        : m_Value(value)
    {}

    std::string EndLog::Value() const
    {
        return m_Value;
    }

    ErrorLog::ErrorLog(DWORD value)
        : m_Value(value)
    {}

    ErrorLog::ErrorLog(const std::error_code& value)
        : m_Value(value.value())
    {}

    DWORD ErrorLog::Value() const
    {
        return m_Value;
    }

    LoggerStream::LoggerStream(bool bEnabled, const std::function<void(const std::wstringstream&)>& endCb /* = nullptr */)
        : m_bEnabled(bEnabled)
        , m_EndCb(endCb)
    {}

    LoggerStream& LoggerStream::operator<<(const std::string& str)
    {
        if (!m_bEnabled)
        {
            return *this;
        }

        std::wstring ws(str.size(), L' ');
        ws.resize(std::mbstowcs(ws.data(), str.c_str(), str.size()));

        m_Stream << ws;
        return *this;
    }

    LoggerStream& LoggerStream::operator<<(const LastErrorLog& lastErrorLog)
    {
        (void)lastErrorLog;

        if (!m_bEnabled)
        {
            return *this;
        }

        *this << "[" << util::GetLastErrorString() << "]";
        return *this;
    }

    LoggerStream& LoggerStream::operator<<(const ErrorLog& errorLog)
    {
        if (!m_bEnabled)
        {
            return *this;
        }

        *this << "[" << util::GetErrorString(errorLog.Value()) << "]";
        return *this;
    }

    void LoggerStream::operator<<(const EndLog& endLog)
    {
        if (!m_bEnabled)
        {
            return;
        }

        *this << endLog.Value();
        m_Stream << std::endl;

        std::wcout << m_Stream.str();

        if (m_EndCb != nullptr)
        {
            m_EndCb(m_Stream);
        }
    }

    void Log::SetLogLevel(LogLevel level)
    {
        s_LogLevel = level;
    }

    void Log::EnableBuildLogging()
    {
        s_bLogBuild = true;
    }

    void Log::DisableBuildLogging()
    {
        s_bLogBuild = false;
    }

    LoggerStream Log::Debug()
    {
        bool bEnabled = IsLogLevelActive(LogLevel::Debug);
        return LoggerStream(bEnabled);
    }

    LoggerStream Log::Info()
    {
        bool bEnabled = IsLogLevelActive(LogLevel::Info);
        return LoggerStream(bEnabled);
    }

    LoggerStream Log::Warning()
    {
        bool bEnabled = IsLogLevelActive(LogLevel::Warning);
        return LoggerStream(bEnabled);
    }

    LoggerStream Log::Error()
    {
        bool bEnabled = IsLogLevelActive(LogLevel::Error);
        return LoggerStream(bEnabled);
    }

    LoggerStream Log::Build()
    {
        return LoggerStream(s_bLogBuild, [](const std::wstringstream& stream) {
            OutputDebugString(stream.str().c_str());
            });
    }

    bool Log::IsLogLevelActive(LogLevel level)
    {
        return static_cast<int>(level) >= static_cast<int>(s_LogLevel);
    }

}

