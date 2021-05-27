/**********************************************************************
Copyright (c) 2020 Advanced Micro Devices, Inc. All rights reserved.
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
********************************************************************/
#pragma once
#include <utility>
// clang-format off
#include "utils/warning_push.h"
#include "utils/warning_ignore_general.h"
//#include "spdlog/sinks/basic_file_sink.h"
//#include "spdlog/sinks/stdout_color_sinks.h"
//#include "spdlog/spdlog.h"
#include "radeonrays.h"
#include "utils/warning_pop.h"
#include <functional>
// clang-format on

namespace rt
{
class Logger
{
    static constexpr char const* LoggerName = "RR logger";

public:
    Logger(const Logger&)  = delete;
    Logger(Logger&&)       = delete;
    Logger&        operator=(const Logger&) = delete;
    Logger&        operator=(Logger&&) = delete;
    static Logger& Get()
    {
        static Logger logger;
        return logger;
    }

    template <typename... Args>
    void Info(Args&&... args);

    template <typename... Args>
    void Warn(Args&&... args);

    template <typename... Args>
    void Error(Args&&... args);

    template <typename... Args>
    void Debug(Args&&... args);

    template <typename... Args>
    void Trace(Args&&... args);

    void SetLogLevel(RRLogLevel log_level) const;
    void SetFileLogger(char const* filename);
    void SetConsoleLogger();

    void SetLoggingCallback(PFN_LoggingCallback Callback);

private:
    void replaceAll(std::string& str, const std::string& from, const std::string& to)
    {
        if (from.empty())
            return;
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos)
        {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();  // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }
	std::string ConvertSPDLogFormatToC(std::string fmt)
	{ 
		replaceAll(fmt, "{}", "%d");
		return fmt;
	}
    template <typename... Args>
    std::string string_format(const std::string& format, Args... args)
    {
        std::string nformat      = ConvertSPDLogFormatToC(format);
        size_t      size    = snprintf(nullptr, 0, nformat.c_str(), args...) + 1;  // Extra space for '\0'
        if (size <= 0)
        {
            return "";
        }
        std::unique_ptr<char[]> buf(new char[size]);
        snprintf(buf.get(), size, nformat.c_str(), args...);
        return std::string(buf.get(), buf.get() + size - 1);  // We don't want the '\0' inside
    }
    template <typename... Args>
    void                               LogMessage(RRLogLevel level, Args&&... args);
    std::function<PFN_LoggingCallback> m_LogCallback = nullptr;
    Logger()
    {
        /*   logger_ = spdlog::stdout_color_mt(LoggerName);
   #ifdef NDEBUG
           logger_->set_level(spdlog::level::info);
   #else
           logger_->set_level(spdlog::level::debug);
   #endif*/
    }
    ~Logger()
    { /* spdlog::shutdown();*/
    }
    /*std::shared_ptr<spdlog::logger> logger_;*/
};

template <typename... Args>
void Logger::LogMessage(RRLogLevel level, Args&&... args)
{
    std::string FinalOutput = string_format(args...);
    if (Logger::Get().m_LogCallback != nullptr)
    {        
        Logger::Get().m_LogCallback(FinalOutput, level);
    }
	else
    {
        printf("%s\n", FinalOutput.c_str());
    }
    // logger_->info(std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::Info(Args&&... args)
{
    LogMessage(RR_LOG_LEVEL_INFO, std::forward<Args>(args)...);
    // logger_->info(std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::Warn(Args&&... args)
{
    LogMessage(RR_LOG_LEVEL_WARN, std::forward<Args>(args)...);
    // logger_->warn(std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::Error(Args&&... args)
{
    LogMessage(RR_LOG_LEVEL_ERROR, std::forward<Args>(args)...);
    // logger_->error(std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::Debug(Args&&... args)
{
    LogMessage(RR_LOG_LEVEL_DEBUG, std::forward<Args>(args)...);
    // logger_->debug(std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::Trace(Args&&... args)
{
    LogMessage(RR_LOG_LEVEL_INFO, std::forward<Args>(args)...);
    // logger_->trace(std::forward<Args>(args)...);
}

}  // namespace rt