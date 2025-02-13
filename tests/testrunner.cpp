#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

#include "../include/logger/bloggerManager.hpp"
#include "../include/logger/blogContext.hpp"
#include "../include/logger/loggers/bconsoleLogger.hpp"
#include "../include/logger/loggers/bfileLogger.hpp"
#include "../include/logger/messages/binaryBMsg.hpp"
#include "../include/logger/decorators/btimestampDecorator.hpp"
#include "../include/logger/decorators/bloglevelDecorator.hpp"
#include "../include/logger/decorators/blocationDecorator.hpp"

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

    std::shared_ptr<BLogger> tmpLogger = BLoggerManager::getLoggerPtr("console");
    assert(tmpLogger != nullptr);
    *tmpLogger << "Success";
    assert(tmpLogger->getLastMessage() == "Success");

    assert(tmpLogger.get() == cLogger && tmpLogger == BLoggerManager::getLoggerPtr("console"));
}

void testBTimestampDecorator() {
    std::cout << "Test Timestamp Decorator: \n";
    std::cout << "Available: " << BLoggerManager::getAvailableLoggers() << "\n";
    
    std::shared_ptr<BLogger> logger = BLoggerManager::getLoggerPtr("console");
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
    
    auto logger = BLoggerManager::getLoggerPtr("file_timestamped");
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

    // settings remained unchanged after failed modifications
    (*console)("database")[BLogLevel::ERROR] << "Should still show (enabled topic unchanged)";
    (*console)("security")[BLogLevel::ERROR] << "Should still NOT show (disabled topic unchanged)";

    (*console)[BLogLevel::ERROR] << "Topic and freeze tests completed";
    (*console)[BLogLevel::ERROR] << "=========================";
}

void testEaseOfUsePt1() {
    BLogger& lg = BLoggerManager::get("console");
    lg[BLogLevel::ERROR] << "Sucess: get was working";

    BLogContext memLgW(BLoggerManager::getLoggerPtr("console_logLvl_leveled_timestamped"),"", BLogLevel::WARNING);

    memLgW << "[DEMO] Running low on Mem";
    memLgW << "Still";
    memLgW.error() << "And now for Real";
    memLgW.raw()[BLogLevel::ERROR] << "Works like this as well";
}

void testConditionalLogging() {
    std::cout << "Test Conditional Logging:\n";

    // Setup logger with decorators
    BLogger* what = new BConsoleLogger("test_logger");
    auto baseLogger = std::shared_ptr<BLogger>(what);
    auto logger = BTimestampDecorator::decorate(baseLogger);

    // Test basic conditions
    bool trueCondition = true;
    bool falseCondition = false;
    
    // Test true condition
    (*logger) % trueCondition << "Should be logged";
    assert(logger->getLastMessage() == "Should be logged");

    // Test false condition
    (*logger)[BLogLevel::INFO] % falseCondition << "Should not be logged";
    assert(logger->getLastMessage() == "");  // Reset Message

    // Test with different log levels
    (*logger)[BLogLevel::DEBUG] % trueCondition << "Debug message";
    assert(logger->getLastMessage() == "Debug message");

    // Test with topics
    (*logger)("TestTopic")[BLogLevel::INFO] % trueCondition << "Topic message";
    assert(logger->getLastMessage() == "Topic message");

    // Test multiple conditions
    (*logger)[BLogLevel::INFO] % trueCondition % trueCondition << "Multiple true";
    assert(logger->getLastMessage() == "Multiple true");

    (*logger)[BLogLevel::INFO] % trueCondition % falseCondition << "Should not log";
    assert(logger->getLastMessage() == "");  // Reset

    // Test with lambda conditions
    int x = 5;
    (*logger)[BLogLevel::INFO] % [&x]() { return x > 3; } << "Lambda true";
    assert(logger->getLastMessage() == "Lambda true");

    (*logger)[BLogLevel::INFO] % [&x]() { return x < 3; } << "Lambda false";
    assert(logger->getLastMessage() == "");  // Reset

    // Test with complex messages
    (*logger)[BLogLevel::INFO] % trueCondition << "Part 1" << " Part 2" << " Part 3";
    assert(logger->getLastMessage() == "Part 1 Part 2 Part 3");

    // Test with BLogMessage types
    (*logger)[BLogLevel::INFO] % trueCondition << BinaryBMsg(42);
    assert(logger->getLastMessage() == "00000000000000000000000000101010");

    // Test with decorators
    try {
        BLOG_AT(logger)[BLogLevel::INFO] % trueCondition << "Location test";
        assert(false);
    } catch(std::runtime_error& err) {
        (*logger) << ("Sucess: Cant Locationlog without Decorator");
    }

    // Test with std::function
    std::function<bool()> trueFunc = []() { return true; };
    std::function<bool()> falseFunc = []() { return false; };
    
    (*logger)[BLogLevel::INFO] % trueFunc << "Function true";
    assert(logger->getLastMessage() == "Function true");

    (*logger)[BLogLevel::INFO] % falseFunc << "Function false";
    assert(logger->getLastMessage() == "");

    // Test with function pointer
    bool (*funcPtr)() = []() -> bool { return true; };
    (*logger)[BLogLevel::INFO] % funcPtr << "Function pointer true";
    assert(logger->getLastMessage() == "Function pointer true");

    // Test nullptr function pointer
    bool (*nullFunc)() = nullptr;
    (*logger)[BLogLevel::INFO] % nullFunc << "Should not log";
    assert(logger->getLastMessage() == "");  // Still empty

    std::cout << "All conditional logging tests passed!\n";
}

int main(int argc, char *argv[]) {
    if(argc != 1) {
        std::cout << "Execute-Parameters are not yet implemented" << std::endl;
        std::cout << "You tried: " << std::endl;
        for(int i = 1; i < argc; i++)
          std::cout << "\t" << argv[i] << "\n";
    }
    testConditionalLogging();

    testBLoggerManager();
    const auto& logger = BLoggerManager::getLoggerPtr("console");
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

    testEaseOfUsePt1();

    auto a = BLocationDecorator::decorate(logger);
    (*a)[BLogLevel::ERROR] << "Test A";
    BLOG_AT(a.get())[BLogLevel::ERROR] << "Will it display a Line?";
    auto b = BTimestampDecorator::decorate(a);
    (*b)[BLogLevel::ERROR] << "Test B";
    auto c = BLoglevelDecorator::decorate(b);
    (*c)[BLogLevel::ERROR] << "Test C";

    BLOG_AT(c.get())[BLogLevel::ERROR] << "Working?";


    for(int ii = 0; ii < 1000000; ii++)
        (*logger)[BLogLevel::INFO] << ii;
    

    return 0;
}
