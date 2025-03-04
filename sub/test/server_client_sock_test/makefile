# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 
LDFLAGS = -lpthread

# Server and Client Target executable names
TARGET1 = server
TARGET2 = client

# Source files
SRCS1 = server_sock_multi_thread_test.cpp
SRCS2 = verbose_client.cpp

# Object files
OBJS1 = $(SRCS1:.cpp=.o)
OBJS2 = $(SRCS2:.cpp=.o)

# Header dependencies
DEPS = ../utils/output.h

# Default rule
all: $(TARGET1) $(TARGET2)

# Rule to create the server executable
$(TARGET1): $(OBJS1)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Rule to create the client executable
$(TARGET2): $(OBJS2)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Rule to compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean Object file (make clean)
#clean:
#	rm -f $(OBJS1) $(OBJS2) 
clean:
	rm -f $(OBJS1) $(OBJS2) $(TARGET1) $(TARGET2)

# Phony targets
.PHONY: all clean
