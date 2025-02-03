#ifndef BFILE_LOGGER_HPP
#define BFILE_LOGGER_HPP

#include <fstream>
#include <filesystem>

#include "../blogger.hpp"

class BFileLogger : public BLogger {
    private:
        std::ofstream file;

    protected:
        inline void log(const std::string& message) override {
            if(file.is_open()) {
                file << message;
                file.flush();
            }
        }

    public:
        inline explicit BFileLogger(const std::string& name, const std::string& filename) : BLogger(name) {
            // Convert to native path format for platform independence
            std::filesystem::path filepath(filename);
            
            // Open with binary mode to handle line endings consistently and app for just appending to file
            file.open(filepath, std::ios::app | std::ios::binary);
            
            if(!file.is_open()) {
                throw std::runtime_error("Could not open log file: " + filepath.string());
            }
        }

        inline ~BFileLogger() {
            if(file.is_open()) {
                file.close();
            }
        }
};

#endif
