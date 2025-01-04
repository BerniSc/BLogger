#ifndef BCONSOLE_LOGGER_HPP
#define BCONSOLE_LOGGER_HPP

#include <iostream>

#include "../blogger.hpp"

class BConsoleLogger : public BLogger {
    protected:
        void log(const std::string& message) override {
            // Print no newline here. Our
            std::cout << message;
            std::cout.flush();
        }
    public:
        explicit BConsoleLogger(const std::string& name) : BLogger(name) {}

};


#endif
