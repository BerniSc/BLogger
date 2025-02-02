#include <cassert>
#include <iostream>
#include <memory>

#include "../include/logger/bloggerManager.hpp"
#include "../include/logger/loggers/bconsoleLogger.hpp"
#include "../include/logger/loggers/bfileLogger.hpp"
#include "../include/logger/messages/binaryBMsg.hpp"
#include "../include/logger/decorators/btimestampDecorator.hpp"

void testBinaryMessage(std::shared_ptr<BLogger> lg) {
    std::cout << "Test Binary Message: \n";

    *lg << BinaryBMsg(42);
    assert(lg->getLastMessage() == "00000000000000000000000000101010");
    *lg << BinaryBMsg('A');
    assert(lg->getLastMessage() == "01000001");
    *lg << BinaryBMsg(42ul);
    assert(lg->getLastMessage() == "0000000000000000000000000000000000000000000000000000000000101010");
    *lg << BinaryBMsg(uint8_t(42));
    assert(lg->getLastMessage() == "00101010");
}

void testBLoggerManager() {
    std::cout << "Test Logger Manager: \n";

    std::string loggers = BLoggerManager::getAvailableLoggers();
    assert(loggers == "");

    BLogger *cLogger = new BConsoleLogger("console");
    BLoggerManager::addLogger(std::shared_ptr<BLogger>(cLogger));
    loggers = BLoggerManager::getAvailableLoggers();
    assert(loggers == "console\n");

    std::shared_ptr<BLogger> tmpLogger = BLoggerManager::getLogger("console");
    assert(tmpLogger != nullptr);
    *tmpLogger << "Success";
    assert(tmpLogger->getLastMessage() == "Success");

    assert(tmpLogger.get() == cLogger && tmpLogger == BLoggerManager::getLogger("console"));

}

void testBTimestampDecorator() {
    std::cout << "Test Timestamp Decorator: \n";
    std::cout << "Available: " << BLoggerManager::getAvailableLoggers() << "\n";
    
    std::shared_ptr<BLogger> logger = BLoggerManager::getLogger("console");
    auto decorated = BTimestampDecorator::decorate(logger);
    BLoggerManager::addLogger(decorated);
    std::cout << logger->getName() << "\n";
    
    assert(decorated->getName() == "console_timestamped" && logger->getName() == "console");
    
    *logger << "Undecorated Log";
    *decorated << "Decorated Log" << " test";
    assert(decorated->getLastMessage() == "Decorated Log test");
    *decorated << BinaryBMsg('A') << " Working? " << BinaryBMsg(42);
    assert(decorated->getLastMessage() == "01000001 Working? 00000000000000000000000000101010");

    std::string lastMsg = logger->getLastMessage();
    *decorated << lastMsg;

    try {
        BTimestampDecorator::decorate(nullptr);
        assert(false);
    } catch(const std::invalid_argument&) {

    }

    auto decorated2 = BTimestampDecorator::decorate(decorated);
    *decorated2 << "Chained";
    lastMsg = decorated2->getLastMessage();
    assert(lastMsg == "Chained");
}

int main(int argc, char *argv[]) {
    if(argc != 1) {
        std::cout << "Execute-Parameters are not yet implemented" << std::endl;
        std::cout << "You tried: " << std::endl;
        for(int i = 1; i < argc; i++)
          std::cout << "\t" << argv[i] << "\n";
    }

    testBLoggerManager();
    const auto& logger = BLoggerManager::getLogger("console");
    testBinaryMessage(logger);

    testBTimestampDecorator();
    
    BLogger *fLogger = new BFileLogger("file", "./log/01_log");
    auto dfLogger = BTimestampDecorator::decorate(std::shared_ptr<BLogger>(fLogger));
    *fLogger << "Test Me";
    *dfLogger << "Me Too";
    
    return 0;
}
