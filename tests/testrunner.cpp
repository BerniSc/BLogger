#include <iostream>
#include <memory>

#include "../include/logger/bloggerManager.hpp"
#include "../include/logger/loggers/bconsoleLogger.hpp"

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
    
    *cLogger << "Test" << "hi" << 42;
    

    return 0;
}
