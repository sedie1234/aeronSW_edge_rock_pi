# Compiler
CXX = g++
CXXFLAGS = -Wall -Wextra -Wimplicit-fallthrough=5 -Iinclude -std=c++17
LDFLAGS = -Wl,-rpath,$(PWD)/third_party/librdkafka -L$(PWD)/third_party/librdkafka -lrdkafka++ -lrdkafka -lpthread

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Output executable
TARGET = $(BIN_DIR)/run

# Source files and object files
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))

# Default target 
all: $(TARGET)

run: $(TARGET)
	./run.sh

debug: $(TARGET)
	./debug.sh

# Create output executable
$(TARGET): $(OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(OBJ_FILES) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete! Executable created: $(TARGET)"

# Compile source files into object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	@rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "Cleaned build files."

# Phony targets
.PHONY: all clean
