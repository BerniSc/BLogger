#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <unordered_set>
#include <vector>

#include "../include/logger/bloggerManager.hpp"
#include "../include/logger/blogContext.hpp"
#include "../include/logger/loggers/bconsoleLogger.hpp"
#include "../include/logger/loggers/bfileLogger.hpp"
#include "../include/logger/messages/binaryBMsg.hpp"
#include "../include/logger/decorators/btimestampDecorator.hpp"
#include "../include/logger/decorators/bloglevelDecorator.hpp"
#include "../include/logger/decorators/blocationDecorator.hpp"

#include "tests.ipp"

struct TestCase {
    std::string name;
    std::function<void()> test;
    std::vector<std::string> dependencies;  // Names of tests that must run before this one
    bool isolate;
};

class TestRunner {
private:
    std::map<std::string, TestCase> tests;
    std::unordered_set<std::string> executedTests;

    void runSingle(const TestCase& test, bool dependency = false) {
        if(executedTests.find(test.name) != executedTests.end()) {
            std::cout << "Skipping already executed test: " << test.name << "\n";
            return;
        }

        std::cout << "\nRunning test: " << test.name << "\n";
        if(test.isolate && !dependency) {
            #ifdef LOGGER_DEBUG
                std::cout << "ISOLATED\n";
                BLoggerManager::debugReset();
                BLoggerConfig::debugReset();
                BLogger::debugReset();
            #endif
            executedTests.clear();
        }

        // Run dependencies first
        for(const auto& dep : test.dependencies) {
            std::cout << "ASKDJH " << dep << "\n";
            if(tests.find(dep) != tests.end()) {
                runSingle(tests[dep], true);
            } else {
                std::cout << "=== FAILED TO FIND " << dep << "\n";
            }
        }

        test.test();
        executedTests.insert(test.name); 
        std::cout << "\nSucceeded test: " << test.name << "\n";
    }

public:
     void addTest(const std::string& name, 
                  std::function<void()> testFunc, 
                  std::vector<std::string> deps = {},
                  bool isolate = false) {
        tests[name] = {name, testFunc, deps, isolate};
    }

    void runAll() {
        for(const auto& [name, test] : tests) {
            std::cout << name << "\n";
            runSingle(test);
        }
    }

    void runTest(const std::string& name) {
        if(tests.find(name) != tests.end())
            runSingle(tests[name]);
    }   
};


int main(int argc, char *argv[]) {
    std::cout << "Setting up Tests\n";
    TestRunner runner;

    runner.addTest("0BinaryMessage", []() {
        auto logger = std::make_shared<BConsoleLogger>("test");
        testBinaryMessage(logger);
    }, {}, false);

    runner.addTest("1LoggerManager", testBLoggerManager, {}, true);
    runner.addTest("2TimestampDecorator", testBTimestampDecorator, {"1LoggerManager"}, true);
    runner.addTest("3ThreadSafety", testThreadSafety, {}, true);
    runner.addTest("4LogLevels", testLogLevels, {"1LoggerManager"}, true);
    runner.addTest("5TopicsAndFreeze", testTopicsAndFreeze, {"1LoggerManager", "4LogLevels"}, true);
    runner.addTest("6EaseOfUse", testEaseOfUsePt1, {"1LoggerManager", "4LogLevels"}, true);
    runner.addTest("7LocationLogger", testLocationLogger, {}, true);

    if(argc != 1) {
        for(int i = 1; i < argc; i++) {
            std::cout << "Tring test " << argv[i] << "\n";  
            runner.runTest(argv[i]);
        }
        return 0;
    }


    runner.runAll();

    return 0;
}
