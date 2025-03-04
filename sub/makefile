# Compiler
CXX = g++
CXXFLAGS = -Wall -Wextra -Wimplicit-fallthrough=5 -Iinclude -std=c++17 
LDFLAGS = -Wl,-rpath,$(PWD)/third_party/librdkafka -L$(PWD)/third_party/librdkafka -lrdkafka++ -lrdkafka -lpthread 

#cam include 
CAM_HEDER=$(PWD)/../object_detection
# CAM_HEDER=$(shell find $(PWD)/../object_detection -name "*.h")
# CXXFLAGS += -I$(CAM_HEDER)/video_test/cpp  -I$(CAM_HEDER)/utils -I$(CAM_HEDER)/3rdparty/timer  -I$(CAM_HEDER)/3rdparty/allocator/dma -I$(CAM_HEDER)/3rdparty/allocator/drm 
LDFLAGS += -L$(PWD)/../build/object_detection/video_test/cpp/3rdparty.out -L$(PWD)/../build/object_detection/video_test/cpp/utils.out -L$(PWD)/../build/object_detection/video_test/cpp/libcam_lib.a
# CXXFLAGS+= $(foreach files, $(CAM_HEDER), -I$(files))

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
