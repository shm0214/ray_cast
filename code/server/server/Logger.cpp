#include "Server/Logger.hpp"

namespace NRenderer {
Logger::Logger() : msgs(), mtx() {
    msgs.reserve(100);
    std::chrono::zoned_time now{"Asia/Shanghai",
                                std::chrono::system_clock::now()};
    auto today = std::chrono::floor<std::chrono::days>(now.get_local_time());
    std::string datetime =
        std::format("../../../log/{:%Y_%m_%d}_{:%H_%M_%S}.log",
                    std::chrono::year_month_day{today},
                    std::chrono::hh_mm_ss{now.get_local_time() - today});
    logFile.open(datetime, ios::out);
}
void Logger::log(const string& msg, LogType type) {
    mtx.lock();
    auto t = chrono::system_clock::now();
    auto time = chrono::system_clock::to_time_t(t);
    string timeStr{ctime(&time)};
    timeStr.pop_back();
    string prefix = "[ " + timeStr + " ] ";
    msgs.push_back({type, (prefix + msg)});
    logFile << prefix + msg << endl;
    mtx.unlock();
}
void Logger::log(const string& msg) {
    this->log(msg, LogType::NORMAL);
}
void Logger::warning(const string& msg) {
    this->log(msg, LogType::WARNING);
}
void Logger::error(const string& msg) {
    this->log(msg, LogType::ERROR);
}
void Logger::success(const string& msg) {
    this->log(msg, LogType::SUCCESS);
}
void Logger::clear() {
    mtx.lock();
    msgs.clear();
    mtx.unlock();
}
Logger::LogMessages Logger::get() {
    int maxNums = 50;
    mtx.lock();
    LogMessages logMsg;
    if (msgs.size() < 50) {
        logMsg.nums = msgs.size();
        if (logMsg.nums != 0) {
            logMsg.msgs = &msgs[0];
        } else {
            logMsg.msgs = nullptr;
        }
    } else {
        logMsg.nums = 50;
        logMsg.msgs = &msgs[msgs.size() - 50];
    }
    mtx.unlock();
    return logMsg;
}
}  // namespace NRenderer

NRenderer::Logger& getLogger() {
    static NRenderer::Logger logger{};
    return logger;
}