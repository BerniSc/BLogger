#ifndef BLOCATION_DECORATOR_HPP
#define BLOCATION_DECORATOR_HPP

#include "../decorators/bloggerDecorator.hpp"

#define BLOG_AT(logger) \
    ([&]() -> BLogger& { \
        auto* loc = BLocationDecorator::findInChain(BLoggerAccess::getPtr(logger)); \
        if(!loc) throw std::runtime_error("You need to decorate you Logger with a BLocationDecorator for this to Work!"); \
        return (loc->setLocation(__FILE__, __LINE__), *logger); \
    }())

// Unify all BLoggerAccesses (Raw Ptr, Reference and SharedPtr) into a raw ptr
class BLoggerAccess {
    public:
        static BLogger* getPtr(BLogger* ptr) { return ptr; }
        static BLogger* getPtr(BLogger& ref) { return &ref; }
        static BLogger* getPtr(const std::shared_ptr<BLogger>& ptr) { return ptr.get(); }
};

class BLocationDecorator : public BLoggerDecorator {
    private:
        std::mutex locationMutex;

        struct LocationInfo {
            std::string file;
            int line;
        } currentLocation;
        bool hasLocation = false;

        // Cache last known instance per thread for performance
        static inline thread_local BLocationDecorator* lastInstance = nullptr;
        static inline thread_local BLogger* lastLogger = nullptr;

    protected:
        std::string decorateMessage(const std::string& msg) override {
            std::lock_guard<std::mutex> lock(locationMutex);
            if(hasLocation) {
                hasLocation = false;
                return "[" + currentLocation.file + ":" + std::to_string(currentLocation.line) + "] " + msg;
            }
            return msg;
        }


    public:
        BLocationDecorator(std::shared_ptr<BLogger> logger) 
            : BLoggerDecorator(std::move(logger), "location") {
                if(!wrapped)
                    throw std::invalid_argument("Logger cannot be null");
            }

        inline static std::shared_ptr<BLogger> decorate(std::shared_ptr<BLogger> logger) {
            return std::make_shared<BLocationDecorator>(std::move(logger));
        }

        // Set the Location. Called via the Macro. Returns Reference for Chaining
        BLogger& setLocation(const char* file, int line) {
            std::lock_guard<std::mutex> lock(locationMutex);
            currentLocation.file = file;
            currentLocation.line = line;
            hasLocation = true;
            return *this;
        }

        // Chain traversal witth caching
        static BLocationDecorator* findInChain(BLogger* logger) {
            // same logger as last time? Cache Hit
            if(lastInstance && lastLogger == logger)
                return lastInstance;

            // cache miss
            while(logger) {
                if(auto loc = dynamic_cast<BLocationDecorator*>(logger)) {
                    // Update cache
                    lastInstance = loc;
                    lastLogger = logger;
                    return loc;
                }
                if(auto dec = dynamic_cast<BLoggerDecorator*>(logger))
                    logger = dec->getWrappedLogger().get();
                else
                    break;
            }
            
            // Not found
            lastInstance = nullptr;
            lastLogger = nullptr;
            return nullptr;
        }
};

#endif
