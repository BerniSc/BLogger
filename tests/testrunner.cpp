#include <cassert>
#include <iostream>
#include <memory>

#include "../include/logger/bloggerManager.hpp"
#include "../include/logger/loggers/bconsoleLogger.hpp"
#include "../include/logger/messages/binaryBMsg.hpp"

void testBinaryMessage(BLogger& lg) {
    std::cout << "Test Binary Message: \n";

    lg << BinaryBMsg(42);
    assert(lg.getLastMessage() == "00000000000000000000000000101010");
    lg << BinaryBMsg('A');
    assert(lg.getLastMessage() == "01000001");
    lg << BinaryBMsg(42ul);
    assert(lg.getLastMessage() == "0000000000000000000000000000000000000000000000000000000000101010");
    lg << BinaryBMsg(uint8_t(42));
    assert(lg.getLastMessage() == "00101010");
}

int main(int argc, char *argv[]) {
    if(argc != 1) {
        std::cout << "Execute-Parameters are not yet implemented" << std::endl;
        std::cout << "You tried: " << std::endl;
        for(int i = 1; i < argc; i++)
          std::cout << "\t" << argv[i] << "\n";
    }

    BLoggerManager *manager = BLoggerManager::getManagerInstance();
    BLogger *cLogger = new BConsoleLogger("console");

    manager->addLogger(std::shared_ptr<BLogger>(cLogger));
    
    int i = 42;

    testBinaryMessage(*cLogger);
    

    *cLogger << "Test" << "hi" << 42;
    *cLogger << BinaryBMsg(i);
    std::cout << "\nTest2\n";
    *cLogger << "Test" << "hi" << i;
    

    return 0;
}
