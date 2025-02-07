#ifndef BLOGGER_STATES_HPP
#define BLOGGER_STATES_HPP

#include <map>
#include <stdexcept>
#include <string>

enum class BLogLevel {
    NONE,
    DEBUG,
    LOG,
    INFO,
    WARNING,
    ERROR
};

const std::map<BLogLevel, std::string> LEVEL_TO_STRING = {
    {BLogLevel::NONE, "NONE"},
    {BLogLevel::DEBUG, "DEBUG"},
    {BLogLevel::LOG, "LOG"},
    {BLogLevel::INFO, "INFO"},
    {BLogLevel::WARNING, "WARNING"},
    {BLogLevel::ERROR, "ERROR"}
};

// Global level management
class BLogLevelManager {
    private:
        // What Loglevels should be permitted? Allow all by Default
        static inline BLogLevel defaultLogLevel = BLogLevel::NONE;

        static std::map<std::string, BLogLevel>& customLevels() {
            static std::map<std::string, BLogLevel> instance;
            return instance;
        }

    public:
        static void setDefaultLogLevel(BLogLevel level) {
            defaultLogLevel = level;
        }

        static void setLoggerLevel(const std::string& loggerName, BLogLevel level) {
            if(customLevels().find(loggerName) != customLevels().end())
                throw std::runtime_error("Log level for '" + loggerName + "' already set: " + LEVEL_TO_STRING.at(level));

            customLevels()[loggerName] = level;
        }

        static BLogLevel getLoggerLevel(const std::string& loggerName) {
            auto& levels = customLevels();
            auto it = levels.find(loggerName);
            return (it != levels.end()) ? it->second : defaultLogLevel;
        }
};

#endif
