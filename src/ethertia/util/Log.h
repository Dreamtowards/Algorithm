//
// Created by Dreamtowards on 2022/4/22.
//

#pragma once

#include <string_view>
#include <format>
#include <iostream>
#include <mutex>


class Log
{
public:


    /// produce a "log_head" e.g. "[2023-07-06.13:44:03.623.765][0x7ff850a73640/INFO]: "
    static void log_head(std::ostream& out, const char* level = "INFO", const char* srcloc = "");

    /// this function is global Synchronous. so wouldn't cause 'Log Tearing' when multiple thread logging at the same time.
    /// @pat if last char is '\1', then '\n' will not be automatically add at the end. (and the '\1' also wouldn't be print out.)
    ///      this is useful when u need append sth. e.g. BenchmarkTimer's time result.
    /// @lv  level text in the 'log_head' e.g. "DEBUG/INFO/WARN/ERROR"
    template<typename... ARGS>
    static void log(std::ostream& out, const char* level, const char* srcloc, std::string_view fmt, ARGS... args)
    {
        static std::mutex g_LogLock;
        std::lock_guard<std::mutex> _guard(g_LogLock);  // prevents multiple logging in same time. have performance issue.

        bool keepline = fmt.back() == '\1';

        //Log::log_head(ss, level, srcloc);
        out << std::vformat(keepline ? fmt.substr(0, fmt.length()-1) : fmt,
                            std::make_format_args(args...));

        if (!keepline) {
            out << std::endl;
        }
    }


    template<typename... ARGS>
    static void info(std::string_view fmt, ARGS... args)
    {
        Log::log(std::cout, "INFO", "", fmt, args...);
    }

    template<typename... ARGS>
    static void warn(std::string_view fmt, ARGS... args)
    {
        Log::log(std::cout, "WARN", "", fmt, args...);
    }


};

