CXX := g++
CXXSTD := -std=c++17
WARN := -Wall -Wextra -Wpedantic
INCLUDES := -Iinclude
PTHREAD := -pthread

BUILD_DIR := build
BIN_DIR := bin

LIB_NAME := liblogger.so
LIB_PATH := $(BUILD_DIR)/$(LIB_NAME)

APP_NAME := logger_demo
APP_PATH := $(BIN_DIR)/$(APP_NAME)

.PHONY: all lib app test clean

all: lib app

lib: $(LIB_PATH)

$(LIB_PATH): src/Logger.cpp include/Logger.h | $(BUILD_DIR)
	$(CXX) $(CXXSTD) $(WARN) $(INCLUDES) -fPIC -shared -o $(LIB_PATH) src/Logger.cpp

app: $(APP_PATH)

$(APP_PATH): app/main.cpp app/MessageQueue.h $(LIB_PATH) | $(BIN_DIR)
	$(CXX) $(CXXSTD) $(WARN) $(INCLUDES) $(PTHREAD) -o $(APP_PATH) app/main.cpp \
		-L$(BUILD_DIR) -llogger -Wl,-rpath,'$$ORIGIN/../$(BUILD_DIR)'

test: app
	LD_LIBRARY_PATH=$(BUILD_DIR) ./$(APP_PATH) app.log info

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) *.log
