#ifndef BTIMESTAMP_DECORATOR_HPP
#define BTIMESTAMP_DECORATOR_HPP

#include <memory>
#include <chrono>
#include <stdexcept>
#include <string>

#include "bloggerDecorator.hpp"

class BTimestampDecorator : public BLoggerDecorator {
    private:
        std::string format;

        bool parseFormat() const;

        std::string getFTimeStamp() const {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);

            char buffer[100];
            
            if(std::strftime(buffer, sizeof(buffer), format.c_str(), std::localtime(&time)))
                return "[" + std::string(buffer) + "] ";
            return "[TimeError]";
        };

    protected:
        std::string decorateMessage(const std::string& msg) override {
            std::string timestamp = getFTimeStamp();
            return timestamp + msg;
        }

    public:
        BTimestampDecorator(std::shared_ptr<BLogger> logger, std::string timeFormat = "%Y-%m-%d %H:%M:%S")
            : BLoggerDecorator(std::move(logger), "timestamped"), 
            format(std::move(timeFormat)) {
                if(!wrapped)
                    throw std::invalid_argument("Logger cannot be null");
            }

        static std::shared_ptr<BLogger> decorate(std::shared_ptr<BLogger> logger, const std::string& format = "%Y-%m-%d %H:%M:%S") {
            if(logger == nullptr)
                throw std::invalid_argument("Loggar cannot be null");
            return std::make_shared<BTimestampDecorator>(std::move(logger), format);
        }
};

#endif
