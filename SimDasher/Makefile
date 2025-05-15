# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall
LDFLAGS = -lws2_32

# Directories
SRC_DIR = src
BIN_DIR = bin

# Source and output
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
TARGET = $(BIN_DIR)/main

# Default target
all: $(TARGET)

# Link objects into executable
$(TARGET): $(SOURCES)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Clean build
clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean
