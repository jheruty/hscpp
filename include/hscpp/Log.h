#pragma once

#include <string>
#include <sstream>
#include <functional>

#include "hscpp/Platform.h"

#define HSCPP_LOG_PREFIX __func__ << "[" << __LINE__ << "]: "

namespace hscpp { namespace log
{
    //============================================================================
    // Level
    //============================================================================

    enum class Level
    {
        Debug,
        Info,
        Warning,
        Error,
    };

    //============================================================================
    // Shorthands
    //============================================================================

    // End the current log stream.
    class End
    {
    public:
        End() = default;;
        explicit End(const std::string& str);

        std::string Str() const;

    private:
        std::string m_Str;
    };

    // Insert the last OS error.
    class LastOsError
    {};

    // Insert the OS error, given the error code.
    class OsError
    {
    public:
        explicit OsError(TOsError errorCode);
        explicit OsError(const std::error_code& errorCode);

        TOsError ErrorCode() const;

    private:
        TOsError m_ErrorCode;
    };

    //============================================================================
    // Stream
    //============================================================================

    class Stream
    {
    public:
        explicit Stream(bool bEnabled, const std::function<void(const std::wstringstream&)>& endCb = nullptr);
        Stream& operator<<(const std::string& str);

        Stream& operator<<(const LastOsError& lastError);
        Stream& operator<<(const OsError& errorLog);
        void operator<<(const End& endLog);

        template <typename T>
        Stream& operator<<(const T& val)
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
        std::function<void(const std::wstringstream&)> m_EndCb;

        std::wstringstream m_Stream;
    };

    //============================================================================
    // Logging Functions
    //============================================================================
    Stream Debug();
    Stream Info();
    Stream Warning();
    Stream Error();
    Stream Build();

    void SetLevel(Level level);

    void EnableBuildLogging();
    void DisableBuildLogging();
}}