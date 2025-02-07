#ifndef BLOGGER_DECORATOR_HPP
#define BLOGGER_DECORATOR_HPP

#include "../blogger.hpp"
#include <memory>

class BLoggerDecorator : public BLogger {
    protected:
        std::shared_ptr<BLogger> wrapped;
        std::string currentMessage;         // Accumulate Message until final "dispatch"

        // Provide an interface to have an arbitrarily decorated Message (Front as well as Back) while collecting the logic here
        virtual std::string decorateMessage(const std::string& msg) = 0;

        inline BLoggerDecorator(std::shared_ptr<BLogger> logger, const std::string& decoratorType)
            : BLogger(logger->getName() + "_" + decoratorType)
            , wrapped(std::move(logger)) {
            if(!wrapped) {
                throw std::invalid_argument("Logger cannot be null");
            }
        }

        inline void log(const std::string& msg) override {
            // "Control Message" was sent -> If not empty clean up current message to be ready for next log
            if(msg == "\n") {
                if(!currentMessage.empty()) {
                    wrapped->log(decorateMessage(currentMessage));
                    currentMessage.clear();
                }
                wrapped->log(msg);
            } else {
                currentMessage += msg;
            }
        }
    
    public:
        // In a Decorator we want to forward the GetLastMessage to the downmost 
        // Instance (the original Logger) to get the real Message
        inline const std::string& getLastMessage() const override {
            return wrapped->getLastMessage();
        }

        // Also override operator[] to propagate to wrapped logger
        BLogger& operator[](BLogLevel level) override {
            wrapped->operator[](level);  // Set level on wrapped logger
            return *this;
        }

        // Also override getLogLevel to get from wrapped logger
        BLogLevel getLogLevel() const override {
            return wrapped->getLogLevel();
        }

};

#endif
