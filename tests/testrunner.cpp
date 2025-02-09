#include <cassert>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "../include/logger/bloggerManager.hpp"
#include "../include/logger/loggers/bconsoleLogger.hpp"
#include "../include/logger/loggers/bfileLogger.hpp"
#include "../include/logger/messages/binaryBMsg.hpp"
#include "../include/logger/decorators/btimestampDecorator.hpp"
#include "../include/logger/decorators/bloglevelDecorator.hpp"

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
    } catch(const std::invalid_argument&) { }

    auto twiceDecorated = BTimestampDecorator::decorate(decorated);
    *twiceDecorated << "Chained";
    lastMsg = twiceDecorated->getLastMessage();
    assert(lastMsg == "Chained");
}

void testThreadSafety() {
    std::cout << "Testing Thread Safety:\n";
    
    const int NUM_THREADS = 10;
    const int MSGS_PER_THREAD = 1000;
    
    auto logger = BLoggerManager::getLogger("file_timestamped");
    // auto decorated = BTimestampDecorator::decorate(logger);
    auto decorated = logger;
    
    std::vector<std::thread> threads;
    std::atomic<int> counter{0};

    // Create multiple threads that log simultaneously
    for(int i = 0; i < NUM_THREADS; i++) {
        threads.emplace_back([&decorated, &counter, i, MSGS_PER_THREAD]() {
            for(int j = 0; j < MSGS_PER_THREAD; j++) {
                *decorated << "Thread " << i << " Message " << j;
                counter++;
            }
        });
    }

    // Wait for all threads to complete
    for(auto& thread : threads) {
        thread.join();
    }

    // Verify results
    assert(counter == NUM_THREADS * MSGS_PER_THREAD);
    
    std::cout << "Thread safety test completed. Check logs for corruption.\n";
}

void testLogLevel(std::shared_ptr<BLogger> lg) {
    lg = BLoglevelDecorator::decorate(lg);

    *lg << "This should have a level";
    (*lg)[BLogLevel::DEBUG] << "This one too";
    (*lg)[BLogLevel::INFO] << "Now Info";
    
    lg = BTimestampDecorator::decorate(lg);

    (*lg)[BLogLevel::WARNING] << "Now Warning";
    (*lg)[BLogLevel::LOG] << "Now Log";
    (*lg)[BLogLevel::ERROR] << "Now Error";
}

void testLogLevels() {
    auto console_logLvl = BLoglevelDecorator::decorate(std::make_shared<BConsoleLogger>("console_logLvl"));
    BLoggerManager::addLogger(console_logLvl);
    BLoggerConfig::setTopics({"console"});
    (*console_logLvl)[BLogLevel::ERROR] << "=========================";
    
    BLoggerConfig::setDefaultLogLevel(BLogLevel::NONE);
    *console_logLvl << "Should show the Default (NONE)";  // NONE >= NONE 
    (*console_logLvl)[BLogLevel::ERROR] << "Should show (ERROR)";  // ERROR >= NONE
    (*console_logLvl)[BLogLevel::DEBUG] << "Should show (DEBUG)";  // DEBUG >= NONE
    (*console_logLvl)[BLogLevel::INFO] << "Should show (INFO)";  // INFO >= NONE
    

    BLoggerConfig::setDefaultLogLevel(BLogLevel::WARNING);
    *console_logLvl << "Should not show the Default (NONE)";  // NONE <= WARNING
    (*console_logLvl)[BLogLevel::ERROR] << "Should show (ERROR)";  // ERROR >= WARNING
    (*console_logLvl)[BLogLevel::DEBUG] << "Should not show (DEBUG)";  // DEBUG <= WARNING
    (*console_logLvl)[BLogLevel::INFO] << "Should not show (INFO)";  // INFO <= WARNING

    BLoggerConfig::setLoggerLevel("console_logLvl", BLogLevel::DEBUG);
    (*console_logLvl)[BLogLevel::INFO] << "Should show (INFO)";  // Should show as INFO >= DEBUG
    
    try {
        BLoggerConfig::setLoggerLevel("console_logLvl", BLogLevel::ERROR);
        std::cout << "Test failed: Should not allow second level setting" << std::endl;
        assert(false);
    } catch(const std::runtime_error&) {
        std::cout << "Test passed: Cannot set level twice" << std::endl;
    }
    
    auto decorated = BTimestampDecorator::decorate(console_logLvl);
    BLoggerManager::addLogger(decorated);
    
    (*decorated)[BLogLevel::INFO] << "Should show with timestamp (INFO)";  // Should show with timestamp
    (*decorated)[BLogLevel::DEBUG] << "Should show with timestamp (DEBUG)";  // Should show with timestamp
    
    auto fileLogger_logLvl = std::make_shared<BFileLogger>("file_logLvl", "./log/02_log");
    BLoggerManager::addLogger(fileLogger_logLvl);
    BLoggerConfig::setLoggerLevel("file_logLvl", BLogLevel::ERROR);
    
    (*fileLogger_logLvl)[BLogLevel::WARNING] << "Should not show (WARNING)";  // Shouldn't show as WARNING < ERROR
    (*fileLogger_logLvl)[BLogLevel::ERROR] << "Should show (ERROR)";  // Should show as ERROR >= ERROR
    
    std::cout << "Log level tests completed" << std::endl;
}

void testTopicsAndFreeze() {
    auto console = BLoglevelDecorator::decorate(std::make_shared<BConsoleLogger>("console"));
    BLoggerManager::addLogger(console);
    (*console)[BLogLevel::ERROR] << "=========================";
    (*console)[BLogLevel::ERROR] << "Testing Topics and Freeze";
    
    // Default behavior (no topics set)
    (*console)[BLogLevel::INFO] << "Should show (no topics set)";
    (*console)("random")[BLogLevel::INFO] << "Should show (no topics set)";

    // Topic filtering
    BLoggerConfig::setTopics({"memorymanagement", "logging"});
    *console << "Should show (no topic)";
    (*console)("memorymanagement")[BLogLevel::INFO] << "Should show (enabled topic)";
    (*console)("logging")[BLogLevel::INFO] << "Should show (enabled topic)";
    (*console)("drivers")[BLogLevel::ERROR] << "Should NOT show (disabled topic)";

    // Topic and level interaction
    BLoggerConfig::setDefaultLogLevel(BLogLevel::WARNING);
    (*console)("memorymanagement")[BLogLevel::INFO] << "Should NOT show (enabled topic, but INFO < WARNING)";
    (*console)("memorymanagement")[BLogLevel::ERROR] << "Should show (enabled topic and ERROR >= WARNING)";
    (*console)("drivers")[BLogLevel::ERROR] << "Should NOT show (disabled topic, even with ERROR)";

    // Freeze
    BLoggerConfig::freeze();

    try {
        BLoggerConfig::setTopics({"new_topic"});
        std::cout << "Test failed: Should not allow topic changes after freeze" << std::endl;
        assert(false);
    } catch(const std::runtime_error&) {
        (*console)[BLogLevel::INFO] << "Test passed: Cannot change topics after freeze";
    }

    try {
        BLoggerConfig::setDefaultLogLevel(BLogLevel::ERROR);
        std::cout << "Test failed: Should not allow level changes after freeze" << std::endl;
        assert(false);
    } catch(const std::runtime_error&) {
        (*console)[BLogLevel::INFO] << "Test passed: Cannot change default level after freeze";
    }

    try {
        BLoggerConfig::setLoggerLevel("console", BLogLevel::ERROR);
        std::cout << "Test failed: Should not allow custom level changes after freeze" << std::endl;
        assert(false);
    } catch(const std::runtime_error&) {
        (*console)[BLogLevel::INFO] << "Test passed: Cannot set custom levels after freeze";
    }

    // Test 5: Verify settings remained unchanged after failed modifications
    (*console)("database")[BLogLevel::ERROR] << "Should still show (enabled topic unchanged)";
    (*console)("security")[BLogLevel::ERROR] << "Should still NOT show (disabled topic unchanged)";

    (*console)[BLogLevel::ERROR] << "Topic and freeze tests completed";
    (*console)[BLogLevel::ERROR] << "=========================";
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
    BLoggerManager::addLogger(std::shared_ptr<BLogger>(dfLogger));
    *fLogger << "Test Me";
    *dfLogger << "Me Too";

    *logger << BLoggerManager::getAvailableLoggers();

    testThreadSafety();

    testLogLevel(logger);

    testLogLevels();

    testTopicsAndFreeze();
    
    return 0;
}
