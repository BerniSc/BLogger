#include <iostream>
#include <memory>

#include "../include/logger/bloggerManager.hpp"
#include "../include/logger/loggers/bconsoleLogger.hpp"
#include "../include/logger/messages/binaryBMsg.hpp"

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


    *cLogger << "Test" << "hi" << 42;
    *cLogger << BinaryBMsg(i);
    std::cout << "\nTest2\n";
    *cLogger << "Test" << "hi" << i;
    

    return 0;
}
