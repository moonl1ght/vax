#pragma once

#include "fsLogger.h"
#include <iostream>
#include <memory>
#include <sstream>

#define LOG_INFO(message) vax::dev::Logger::getInstance().info(message)
#define LOG_ERROR(message) vax::dev::Logger::getInstance().error(message)
#define LOG_WARNING(message) vax::dev::Logger::getInstance().warning(message)
#define LOG_DEBUG(message) vax::dev::Logger::getInstance().debug(message)

namespace vax::dev {
class Logger {
  public:
    enum class Mode {
        CONSOLE,
        FILE,
        BOTH,
    };

    enum class Level {
        FAILURE = 0,
        SUCCESS = 1,
        ERROR = 2,
        INFO = 3,
        WARNING = 4,
        DEBUG = 5,
    };

    enum class LevelSeverity {
        LOW = 0,
        MEDIUM = 1,
        HIGH = 2,
        ONLY_FATAL = 3,
    };

    static constexpr LevelSeverity severity(Level level) {
        switch (level) {
        case Level::FAILURE:
            return LevelSeverity::ONLY_FATAL;
        case Level::SUCCESS:
            return LevelSeverity::HIGH;
        case Level::ERROR:
            return LevelSeverity::HIGH;
        case Level::INFO:
            return LevelSeverity::MEDIUM;
        case Level::WARNING:
            return LevelSeverity::MEDIUM;
        case Level::DEBUG:
            return LevelSeverity::LOW;
        }
        return LevelSeverity::LOW;
    }

#ifdef VAX_LOG_LEVEL
    static constexpr bool hasGlobalLogLevel = true;
    static constexpr Level globalLogLevel = Level::VAX_LOG_LEVEL;
#else
    static constexpr bool hasGlobalLogLevel = false;
    static constexpr Level globalLogLevel = Level::DEBUG;
#endif

    std::string system = "";

    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    Logger() = default;

    Logger(const std::string system)
        : system(system) {};

    Logger(const std::string system, const std::string path)
        : system(system)
        , _fsLogger(std::make_shared<FsLogger>(path)) {};

    ~Logger() = default;

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    Logger(Logger&&) = default;
    Logger& operator=(Logger&&) = default;

    template <typename... Args> void success(const Args&... args) const { log<Level::SUCCESS>(args...); }

    template <typename... Args> void info(const Args&... args) const { log<Level::INFO>(args...); }

    template <typename... Args> void error(const Args&... args) const { log<Level::ERROR>(args...); }

    template <typename... Args> void failure(const Args&... args) const { log<Level::FAILURE>(args...); }

    template <typename... Args> void warning(const Args&... args) const { log<Level::WARNING>(args...); }

    template <typename... Args> void debug(const Args&... args) const { log<Level::DEBUG>(args...); }

    void setLogLevel(Level logLevel) { _logLevel = logLevel; }

    Level getLogLevel() const { return _logLevel; }

    Level effectiveLogLevel() const { return hasGlobalLogLevel ? globalLogLevel : _logLevel; }

    bool shouldLog(Level level) const { return severity(level) >= severity(effectiveLogLevel()); }

    static const char* label(Level level) {
        switch (level) {
        case Level::FAILURE:
            return "💥 FAILURE";
        case Level::ERROR:
            return "❌ ERROR";
        case Level::SUCCESS:
            return "✅ SUCCESS";
        case Level::INFO:
            return "🔵 INFO";
        case Level::WARNING:
            return "🟡 WARNING";
        case Level::DEBUG:
            return "🔧 DEBUG";
        }
        return "UNKNOWN";
    }

    void setMode(Mode mode) { _mode = mode; }

    Mode getMode() const { return _mode; }

    void setPath(const std::string& path) { _fsLogger = std::make_shared<FsLogger>(path); }

    void setFsLogger(std::shared_ptr<FsLogger> fsLogger) { _fsLogger = fsLogger; }

    std::shared_ptr<FsLogger> getFsLogger() const { return _fsLogger; }

  private:
    std::shared_ptr<FsLogger> _fsLogger;
    Mode _mode = Mode::CONSOLE;
    Level _logLevel = Level::DEBUG;

    template <Level level, typename... Args> void log(const Args&... args) const {
        if constexpr (hasGlobalLogLevel) {
            if constexpr (severity(level) >= severity(globalLogLevel)) {
                _write(level, args...);
            }
        } else if (severity(level) >= severity(_logLevel)) {
            _write(level, args...);
        }
    }

    template <typename... Args> void _write(Level level, const Args&... args) const {
        std::ostringstream ss;
        ss << "[" << label(level) << "] ";
        if (!system.empty()) {
            ss << "[" << system << "] ";
        }
        std::string separator = "";
        (((ss << separator << args), separator = ""), ...);
        if (_mode == Mode::CONSOLE || _mode == Mode::BOTH) {
            std::cout << ss.str() << std::endl;
        }
        if (_mode == Mode::FILE || _mode == Mode::BOTH) {
            if (_fsLogger) {
                _fsLogger->log(ss.str());
            } else {
                std::cerr << "No file path set for file logger." << std::endl;
            }
        }
    }
};
} // namespace vax::dev