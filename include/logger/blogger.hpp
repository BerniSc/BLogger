#ifndef BLOGGER_HPP
#define BLOGGER_HPP

#include <cstdint>
#include <mutex>
#include <sstream>
#include <string>
#include <atomic>

#include "bloggerConfig.hpp"
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
            name(std::move(loggerName)), instanceID(++instance_counter) { }

        const std::string name;
        const ID instanceID;

        static inline BLogLevel currentLogLevel = BLogLevel::NONE;
        static inline BLogLevel defaultLogLevel = BLogLevel::NONE;

        // Log last logged Message without decorations. Thread-Local should be sufficient 
        // as thats what we expect anyway most of the Time. 
        static thread_local std::string lastMessage;

        static std::atomic<ID> instance_counter;

        virtual void log(const std::string&) = 0;

    private:
        static inline std::mutex outputMutex;
        static inline std::string topic = "";

        inline bool shouldLog(BLogLevel messageLevel, const std::string& messageTopic) {
            if(!messageTopic.empty() && !BLoggerConfig::isTopicEnabled(messageTopic))
                return false;

            return messageLevel >= BLoggerConfig::getLoggerLevel(name);
        }

        class Chain {
            BLogger& logger;
            std::unique_lock<std::mutex> lock;      // Lock for full Lifecycle of Chain

            bool doLog;

            public:
                // Initial Message only relevant on first call -> Passing responsibility of Logging 
                // to Chain. Chain starts Lock and then performs Operations. After last Chain it
                // destructs and releases Lock
                Chain(BLogger& l, const std::string& initialMsg = "") : logger(l), lock(outputMutex), doLog(l.shouldLog(currentLogLevel, BLogger::topic)) {
                    if(doLog)
                        *this << initialMsg;        // Process the Initial Message under the Lock
                }

                Chain(Chain&& other) noexcept : logger(other.logger), lock(std::move(other.lock)) { }

                // Prevent reasignment so Lock etc stays intact
                Chain& operator=(Chain&&) = delete;
                Chain(const Chain&) = delete;
                Chain& operator=(const Chain&) = delete;

                // Finish Log-Entry with a Newline
                ~Chain() { 
                    if(doLog)
                        logger.log("\n");
                    // Reset Topic and Level
                    topic = "";         
                    currentLogLevel = defaultLogLevel;
                }

                // For BLogMessage types
                template<typename T>
                typename std::enable_if<std::is_base_of<BLogMessage, T>::value, Chain&>::type
                operator<<(const T& msg) {
                    if(doLog) {
                        std::string strMsg = msg.serialize();
                        logger.log(strMsg);
                        lastMessage += strMsg;
                    }
                    return *this;
                }

                // For non-BLogMessage types
                template<typename T>
                typename std::enable_if<!std::is_base_of<BLogMessage, T>::value, Chain&>::type
                operator<<(const T& value) {
                    if(doLog) {
                        std::ostringstream ss;
                        ss << value;

                        std::string strMsg = ss.str();
                        logger.log(strMsg);
                        lastMessage += strMsg;
                    }
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

        virtual inline BLogLevel getLogLevel() const {
            return this->currentLogLevel;
        }

        inline const std::string& getTopic() const {
            return this->topic;
        }

        // Log level via []
        virtual BLogger& operator[](BLogLevel level) {
            currentLogLevel = level;
            return *this;
        }

        // Topic via ()
        BLogger& operator()(const std::string& topic) {
            this->topic = topic;
            return *this;
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
            return Chain(*this, strMsg);
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

            // Convert any regular Datatype to String
            std::ostringstream ss;
            ss << value;

            std::string strMsg = ss.str();
            return Chain(*this, strMsg);
        }
};

inline std::atomic<uint8_t> BLogger::instance_counter = 0;
inline thread_local std::string BLogger::lastMessage = "";

#endif
