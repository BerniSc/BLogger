#ifndef BLOGGER_MANAGER
#define BLOGGER_MANAGER

#include <iostream>
#include <map>
#include <memory>
#include <string>

#include "blogger.hpp"

class BLoggerManager {
    private:
        static std::map<std::string, std::shared_ptr<BLogger>> loggers;
        static BLoggerManager* _instance;

    public:
        static BLoggerManager* getManagerInstance() {
            if(_instance == nullptr)
                _instance = new BLoggerManager();

            return _instance;
        }

        static void addLogger(std::shared_ptr<BLogger> logger) {
            loggers[logger->getName()] = logger;
        }

        static void printAvailableLoggers() {
            // TODO Change to logger.log or smthg like that
            for(const auto& logger : loggers)
                std::cout << logger.second->getName() << "\n";
        }

};

inline BLoggerManager* BLoggerManager::_instance = nullptr;
inline std::map<std::string, std::shared_ptr<BLogger>> BLoggerManager::loggers {};

#endif
