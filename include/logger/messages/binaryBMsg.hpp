#ifndef BINARY_B_MSG_HPP
#define BINARY_B_MSG_HPP

#include <bitset>

#include "../bloggerMessage.hpp"

template<typename T>
class BinaryBMsg : public BLogMessage {
    private:
        const T value;
    
    public:
        explicit BinaryBMsg(T v) : value(v) {}
        const std::string serialize() const override {
            constexpr size_t bits = sizeof(T) * 8;
            // return as Binary Format
            return std::bitset<8>(value).to_string(); 
        }
};

#endif
