#ifndef BLOGGER_HPP
#define BLOGGER_HPP

#include <cstdint>
#include <string>

#include "bloggerStates.hpp"

struct BLogger {
    // "Typedefs" for the more "randomly picked" types so changing later is easier
    public:
        using ID = uint8_t;

    protected:
        BLogger(const std::string& loggerName) :
            name(std::move(loggerName)), instanceID(++instance_counter), currentLogLevel(BLogLevel::NONE) {}

        const std::string& name;
        const ID instanceID;

        BLogLevel currentLogLevel;

        static ID instance_counter;

    public:
        BLogger(const BLogger &) = default;
        BLogger(BLogger &&) = delete;
        BLogger &operator=(const BLogger &) = delete;
        BLogger &operator=(BLogger &&) = delete;

        virtual ~BLogger() = default;

        inline const std::string& getName() const {
            return this->name;
        }

        inline const ID getID() const {
            return this->instanceID;
        }

};

inline uint8_t BLogger::instance_counter = 0;

#endif
