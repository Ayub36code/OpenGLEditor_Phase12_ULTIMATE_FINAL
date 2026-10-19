
#pragma once
#include <string>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <mutex>

namespace Editor {

enum class LogLevel { Info, Warning, Error, GL };

class Logger {
public:
    static Logger& Instance() {
        static Logger s;
        return s;
    }

    void Log(LogLevel level, const std::string& msg, const char* file = "", int line = 0) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << "[" << std::put_time(std::localtime(&t), "%H:%M:%S") << "] ";

        switch(level) {
            case LogLevel::Info:    ss << "\033[32m[INFO]\033[0m "; break;
            case LogLevel::Warning: ss << "\033[33m[WARN]\033[0m "; break;
            case LogLevel::Error:   ss << "\033[31m[ERROR]\033[0m "; break;
            case LogLevel::GL:      ss << "\033[35m[GL]\033[0m "; break;
        }
        ss << msg;
        if(file && file[0]) {
            ss << " (" << file << ":" << line << ")";
        }
        std::cout << ss.str() << std::endl;

        m_LastMessages.push_back(ss.str());
        if(m_LastMessages.size() > 200) m_LastMessages.erase(m_LastMessages.begin());
    }

    const std::vector<std::string>& GetLogs() const { return m_LastMessages; }
    void Clear() { m_LastMessages.clear(); }

private:
    std::mutex m_Mutex;
    std::vector<std::string> m_LastMessages;
};

} // namespace

#define LOG_INFO(msg)    Editor::Logger::Instance().Log(Editor::LogLevel::Info, msg, __FILE__, __LINE__)
#define LOG_WARNING(msg) Editor::Logger::Instance().Log(Editor::LogLevel::Warning, msg, __FILE__, __LINE__)
#define LOG_ERROR(msg)   Editor::Logger::Instance().Log(Editor::LogLevel::Error, msg, __FILE__, __LINE__)
#define LOG_GL_ERROR(msg) Editor::Logger::Instance().Log(Editor::LogLevel::GL, msg, __FILE__, __LINE__)

#define GL_CHECK(x) x; { GLenum err; while((err = glGetError()) != GL_NO_ERROR) {     std::stringstream _ss; _ss << #x << " failed: GL Error " << err;     Editor::Logger::Instance().Log(Editor::LogLevel::GL, _ss.str(), __FILE__, __LINE__); } }
