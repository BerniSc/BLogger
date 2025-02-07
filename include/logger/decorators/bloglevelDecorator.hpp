#ifndef BLOGLEVEL_DECORATOR_HPP
#define BLOGLEVEL_DECORATOR_HPP

#include "bloggerDecorator.hpp"

class BLoglevelDecorator : public BLoggerDecorator {
    private:

    protected:
        inline std::string decorateMessage(const std::string& msg) override {
            std::string logLevel = LEVEL_TO_STRING.at(wrapped->getLogLevel());

            return ("[" + logLevel + "] " + msg);
        }

    public:
        inline BLoglevelDecorator(std::shared_ptr<BLogger> logger)
            : BLoggerDecorator(std::move(logger), "leveled") {
                if(!wrapped)
                    throw std::invalid_argument("Logger cannot be null");
            }

        inline static std::shared_ptr<BLogger> decorate(std::shared_ptr<BLogger> logger) {
            if(logger == nullptr)
                throw std::invalid_argument("Logger cannot be null");
            return std::make_shared<BLoglevelDecorator>(std::move(logger));
        }

};


#endif
