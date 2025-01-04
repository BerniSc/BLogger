#ifndef BLOGGER_HPP
#define BLOGGER_HPP

#include <cstdint>
#include <sstream>
#include <string>

#include "bloggerStates.hpp"
#include "bloggerMessage.hpp"

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

        virtual void log(const std::string&) = 0;

    private:
        static bool needsNewline;

        class Chain {
            BLogger& logger;

            public:
                Chain(BLogger& l) : logger(l) {}
                ~Chain() { 
                    logger.log("\n");
                    needsNewline = true;
                }

                template<typename T>
                Chain& operator<<(const T& value) {
                    std::ostringstream ss;
                    ss << value;
                    logger.log(ss.str());
                    return *this;
                }
                
                friend class BLogger; // Allows BLogger to access Chain
        };

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

        /**
         * Template Logic
         * std::is_base_of<TYPE, T>::value --> returns true/false if T is of type TYPE
         * std::enable_if<condition, RETURNTYPE>::type --> returntype is RETURNTYPE if condition == true
         *                                                  otherwise template Subsitution fails
         **/

        // For Types that have an Implementation of the custom BLogMessage-Interface,
        // we call its serialization function and pass to Log
        // Returntype is Chain, a Subclass of BLogger to keep track of a chain and add newline at the end
        // Refernce not possible due to RValue LValue Conversion and Assignment
        // 
        // Does not work any easier, as we might want to chain infinite amounts of "Logging Operations"
        // togehter, but we want to finish them with a newline for clarity but dont know when exactly
        // We also dont want to "See this behaviour from the outside" to hide complexity...
        template<typename T>
        typename std::enable_if<std::is_base_of<BLogMessage, T>::value, Chain>::type
        operator<<(const T& msg) {
            log(msg.serialize());
            return Chain(*this);
        }

        // For Types that DON'T have an Implementation of the custom BLogMessage-Interface,
        // we default to template->stringstream->string and pass to Log
        // Returntype is Chain, a Subclass of BLogger to keep track of a chain and add newline at the end
        // Refernce not possible due to RValue LValue Conversion and Assignment
        // 
        // Does not work any easier, as we might want to chain infinite amounts of "Logging Operations"
        // togehter, but we want to finish them with a newline for clarity but dont know when exactly
        // We also dont want to "See this behaviour from the outside" to hide complexity...
        template<typename T>
        typename std::enable_if<!std::is_base_of<BLogMessage, T>::value, Chain>::type
        operator<<(const T& value) {
            std::ostringstream ss;
            ss << value;
            log(ss.str());
            return Chain(*this);
        }
};

inline uint8_t BLogger::instance_counter = 0;
inline bool BLogger::needsNewline = false;

#endif
