#ifndef BLOGGER_MESSAGE_HPP
#define BLOGGER_MESSAGE_HPP

#include <string>

struct BLogMessage {
    public:
        virtual const std::string& serialize() const = 0;

};

#endif
