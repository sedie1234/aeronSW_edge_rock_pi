# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 
LDFLAGS = -lpthread

# Server and Client Target executable names
TARGET1 = match

# Source files
SRCS1 = matching_test.cpp

# Object files
OBJS1 = $(SRCS1:.cpp=.o)

# Header dependencies
DEPS = ../utils/output.h

# Default rule
all: $(TARGET1)

# Rule to create the server executable
$(TARGET1): $(OBJS1)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Rule to compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS1) $(TARGET1)

# Phony targets
.PHONY: all clean
