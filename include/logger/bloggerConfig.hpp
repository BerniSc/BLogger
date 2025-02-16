#ifndef BLOGGER_CONFIG_HPP
#define BLOGGER_CONFIG_HPP

#include <map>
#include <set>
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

class BLoggerConfig {
    private:
        // What Loglevels should be permitted? Allow all by Default
        static BLogLevel& defaultLogLevel() {
            static BLogLevel defaultLogLevel = BLogLevel::NONE;
            return defaultLogLevel;
        }

        static std::map<std::string, BLogLevel>& customLevels() {
            static std::map<std::string, BLogLevel> instance;
            return instance;
        }

        // Empty means all topics
        static inline std::set<std::string>& enabledTopics() {
            static std::set<std::string> instance;
            return instance;
        };              

        // Changes to Config permitted?
        static inline bool& frozen() {
            static bool frozen = false;
            return frozen;
        }

    public:
        static void initialize(BLogLevel level, std::initializer_list<std::string> initialTopics = {}) {
            if(frozen()) 
                throw std::runtime_error("Configuration already frozen");
            setDefaultLogLevel(level);
            setTopics(initialTopics);
        }

        static void setDefaultLogLevel(BLogLevel level) {
            if(frozen())
                throw std::runtime_error("Configuration already frozen");
            defaultLogLevel() = level;
        }

        static void setTopics(std::initializer_list<std::string> topics) {
            if(frozen())
                throw std::runtime_error("Configuration already frozen");
            enabledTopics().clear();
            enabledTopics().insert(topics.begin(), topics.end());
        }

        static void setLoggerLevel(const std::string& loggerName, BLogLevel level) {
            if(frozen())
                throw std::runtime_error("Configuration already frozen");
            if(customLevels().find(loggerName) != customLevels().end())
                throw std::runtime_error("Log level for '" + loggerName + "' already set: " + LEVEL_TO_STRING.at(level));

            customLevels()[loggerName] = level;
        }

        static BLogLevel getLoggerLevel(const std::string& loggerName) noexcept {
            auto& levels = customLevels();
            auto it = levels.find(loggerName);
            return (it != levels.end()) ? it->second : defaultLogLevel();
        }

        static bool isTopicEnabled(const std::string& topic) noexcept {
            return enabledTopics().empty() || enabledTopics().find(topic) != enabledTopics().end();
        }

        static void freeze() noexcept {
            frozen() = true;
        }

        #ifdef LOGGER_DEBUG
            static void debugReset() {
                defaultLogLevel() = BLogLevel::NONE;
                enabledTopics().clear();
                customLevels().clear();
                frozen() = false;
            }
        #endif
};

#endif
