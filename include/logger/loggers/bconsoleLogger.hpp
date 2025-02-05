#ifndef BCONSOLE_LOGGER_HPP
#define BCONSOLE_LOGGER_HPP

#include <iostream>

#include "../blogger.hpp"

class BConsoleLogger : public BLogger {
    protected:
        inline void log(const std::string& message) override {
            // Print no newline here. Our Base-Logger will take care of it
            std::cout << message;
            std::cout.flush();
        }

    public:
        inline explicit BConsoleLogger(const std::string& name) : BLogger(name) {}

};

#endif
