#ifndef BLOGGER_MANAGER_HPP
#define BLOGGER_MANAGER_HPP

#include <map>
#include <memory>
#include <string>

#include "blogger.hpp"
#include "bloggerStates.hpp"

class BLoggerManager {
    private:

        static std::map<std::string, std::shared_ptr<BLogger>>& loggers() {
            static std::map<std::string, std::shared_ptr<BLogger>> instance;
            return instance;
        }

    public:
        BLoggerManager() = delete;

        static void addLogger(std::shared_ptr<BLogger> logger) {
            loggers()[logger->getName()] = logger;
        }

        static std::shared_ptr<BLogger> getLogger(const std::string& name) {
            auto& map = loggers();
            auto it = map.find(name);
            if(it == map.end())
                throw std::runtime_error("Logger '" + name + "' not found");

            return it->second;
        }

        static std::string getAvailableLoggers() {
            std::string tmp = "";
            for(const auto& logger : loggers())
                tmp += logger.first + "\n";

            return tmp;
        }

        static void setDefaultLogLevel(BLogLevel level) {
            BLogLevelManager::setDefaultLogLevel(level);
        }

        static void setLoggerLevel(const std::string& loggerName, BLogLevel level) {
            BLogLevelManager::setLoggerLevel(loggerName, level);
        }
};

#endif
