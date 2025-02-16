#ifndef BLOG_CONTEXT_HPP
#define BLOG_CONTEXT_HPP

#include <memory>

#include "blogger.hpp"
#include "bloggerConfig.hpp"

class BLogContext {
private:
    std::shared_ptr<BLogger> logger;
    const std::string defaultTopic;
    const BLogLevel defaultLevel;

public:
    BLogContext(std::shared_ptr<BLogger> l, std::string topic, BLogLevel level = BLogLevel::INFO)
        : logger(l), defaultTopic(topic), defaultLevel(level) {}

    template<typename T>
    auto operator<<(const T& msg) {
        return (*logger)(defaultTopic)[defaultLevel] << msg;
    }

    BLogger& raw() {
        return (*logger);
    }

    std::shared_ptr<BLogger>& ptr() {
        return logger;
    }

    BLogger& none() { return (*logger)(defaultTopic)[BLogLevel::NONE]; }
    BLogger& debug() { return (*logger)(defaultTopic)[BLogLevel::DEBUG]; }
    BLogger& log() { return (*logger)(defaultTopic)[BLogLevel::LOG]; }
    BLogger& info() { return (*logger)(defaultTopic)[BLogLevel::INFO]; }
    BLogger& warning() { return (*logger)(defaultTopic)[BLogLevel::WARNING]; }
    BLogger& error() { return (*logger)(defaultTopic)[BLogLevel::ERROR]; }
};

#endif
