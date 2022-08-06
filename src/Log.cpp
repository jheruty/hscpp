#include <iostream>

#include "hscpp/Log.h"
#include "hscpp/Util.h"

namespace hscpp { namespace log
{
    static Level s_Level = Level::Info;
    static bool s_bLogBuild = true;

    End::End(const std::string& str)
        : m_Str(str)
    {}

    std::string End::Str() const
    {
        return m_Str;
    }

    OsError::OsError(TOsError errorCode)
        : m_ErrorCode(errorCode)
    {}

    OsError::OsError(const std::error_code& errorCode)
        : m_ErrorCode(errorCode.value())
    {}

    TOsError OsError::ErrorCode() const
    {
        return m_ErrorCode;
    }

    Stream::Stream(bool bEnabled, const std::function<void(const std::stringstream&)>& endCb /* = nullptr */)
        : m_bEnabled(bEnabled)
        , m_EndCb(endCb)
    {}

    Stream& Stream::operator<<(const std::string& str)
    {
        if (!m_bEnabled || str.empty())
        {
            return *this;
        }

        m_Stream << str;
        return *this;
    }

    Stream& Stream::operator<<(const LastOsError& lastOsError)
    {
        (void)lastOsError;

        if (!m_bEnabled)
        {
            return *this;
        }

        m_Stream << "[" << platform::GetLastErrorString() << "]";
        return *this;
    }

    Stream& Stream::operator<<(const OsError& osError)
    {
        if (!m_bEnabled)
        {
            return *this;
        }

        m_Stream << "[" << platform::GetErrorString(osError.ErrorCode()) << "]";
        return *this;
    }

    void Stream::operator<<(const End& endLog)
    {
        if (!m_bEnabled)
        {
            return;
        }

        *this << endLog.Str();
        std::cout << m_Stream.str() << std::endl;

        if (m_EndCb != nullptr)
        {
            m_EndCb(m_Stream);
        }
    }

    static bool IsLevelActive(Level level)
    {
        return static_cast<int>(level) >= static_cast<int>(s_Level);
    }

    Stream Debug()
    {
        bool bEnabled = IsLevelActive(Level::Debug);
        return Stream(bEnabled);
    }

    Stream Info()
    {
        bool bEnabled = IsLevelActive(Level::Info);
        return Stream(bEnabled);
    }

    Stream Warning()
    {
        bool bEnabled = IsLevelActive(Level::Warning);
        return Stream(bEnabled);
    }

    Stream Error()
    {
        bool bEnabled = IsLevelActive(Level::Error);
        return Stream(bEnabled);
    }

    Stream Build()
    {
        // Direct logs to debugger output.
        return Stream(s_bLogBuild, [](const std::stringstream& stream) {
            platform::WriteDebugString(stream.str());
        });
    }

    void SetLevel(Level level)
    {
        s_Level = level;
    }

    void EnableBuildLogging()
    {
        s_bLogBuild = true;
    }

    void DisableBuildLogging()
    {
        s_bLogBuild = false;
    }

}}

