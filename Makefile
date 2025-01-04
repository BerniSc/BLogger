CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
INCLUDES = -I./include
TARGET = main

# Source files
SRCS = tests/testrunner.cpp
# Object files
OBJS = $(SRCS:.cpp=.o)

# Main target
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

run:
	./$(TARGET)

# Create include directory if it doesn't exist
create_dirs:
	@mkdir -p include

# Clean build files
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean create_dirs run
