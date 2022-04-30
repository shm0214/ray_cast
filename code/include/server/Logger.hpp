#pragma once
#ifndef __NR_LOGGER_HPP__
#define __NR_LOGGER_HPP__

#include <chrono>
#include <ctime>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "common/macros.hpp"

#undef ERROR

namespace NRenderer {
using namespace std;
class DLL_EXPORT Logger {
   public:
    enum class LogType { NORMAL, WARNING, ERROR, SUCCESS };
    struct LogText {
        LogType type;
        string message;
        LogText() = delete;
        LogText(const string& str) : type(LogType::NORMAL), message(str) {}
        LogText(LogType type, const string& str) : type(type), message(str) {}
    };

   private:
    vector<LogText> msgs;
    std::ofstream logFile;
    mutex mtx;

   public:
    Logger();
    ~Logger() { logFile.close(); }
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;

    void log(const string& msg, LogType type);

    void log(const string& msg);

    void warning(const string& msg);

    void error(const string& msg);

    void success(const string& msg);

    void clear();

    struct LogMessages {
        unsigned nums;
        LogText* msgs;
    };

    LogMessages get();
};
using SharedLogger = shared_ptr<Logger>;
}  // namespace NRenderer

#endif