#ifndef BLOGGER_HPP
#define BLOGGER_HPP

#include <cstdint>
#include <sstream>
#include <string>
#include <atomic>

#include "bloggerStates.hpp"
#include "bloggerMessage.hpp"

class BLoggerDecorator;

struct BLogger {
    // Allow the decorator as a friend so we can call the "log" function to prevent a superfluous "newline"
    friend class BLoggerDecorator;

    // "Typedefs" for the more "randomly picked" types so changing later is easier
    public:
        using ID = uint8_t;

    protected:
        BLogger(const std::string& loggerName) :
            name(std::move(loggerName)), instanceID(++instance_counter), currentLogLevel(BLogLevel::NONE) {}

        const std::string name;
        const ID instanceID;

        BLogLevel currentLogLevel;

        // Log last logged Message without decorations. Thread-Local should be sufficient 
        // as thats what we expect anyway most of the Time. 
        static thread_local std::string lastMessage;

        static std::atomic<ID> instance_counter;

        virtual void log(const std::string&) = 0;

    private:
        class Chain {
            BLogger& logger;

            public:
                Chain(BLogger& l) : logger(l) { }

                ~Chain() { 
                    logger.log("\n");
                }

                // For BLogMessage types
                template<typename T>
                typename std::enable_if<std::is_base_of<BLogMessage, T>::value, Chain&>::type
                operator<<(const T& msg) {
                    std::string strMsg = msg.serialize();
                    logger.log(strMsg);
                    lastMessage += strMsg;

                    return *this;
                }

                // For non-BLogMessage types
                template<typename T>
                typename std::enable_if<!std::is_base_of<BLogMessage, T>::value, Chain&>::type
                operator<<(const T& value) {
                    std::ostringstream ss;
                    ss << value;

                    std::string strMsg = ss.str();
                    logger.log(strMsg);
                    lastMessage += strMsg;

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

        inline ID getID() const {
            return this->instanceID;
        }

        virtual inline const std::string& getLastMessage() const {
            return this->lastMessage;
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
            // We enter this Function every FIRST Entry of a Log. Therefore we can reset the old "LastMsg"
            lastMessage = "";

            std::string strMsg = msg.serialize();
            log(strMsg);
            lastMessage += strMsg;

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
            // We enter this Function every FIRST Entry of a Log. Therefore we can reset the old "LastMsg"
            lastMessage = "";

            std::ostringstream ss;
            ss << value;

            std::string strMsg = ss.str();
            log(strMsg);
            lastMessage += strMsg;

            return Chain(*this);
        }
};

inline std::atomic<uint8_t> BLogger::instance_counter = 0;
inline thread_local std::string BLogger::lastMessage = "";

#endif
