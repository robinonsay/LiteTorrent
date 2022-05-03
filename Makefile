# Credit: https://spin.atomicobject.com/2016/08/26/makefile-c-projects/
SRC_DIR ?= ./src
TRACKER_DIR ?= $(SRC_DIR)/tracker
PEER_DIR ?= $(SRC_DIR)/peer
TEST_DIR ?= $(SRC_DIR)/tests
BUILD_DIR ?= ./bin
INC_DIR ?= ./include

SRCS := $(shell ls $(SRC_DIR)/*.cpp)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o) 
DEPS := $(shell find $(INC_DIR) -name *.h)

TRACKER_SRCS := $(shell find $(TRACKER_DIR) -name *.cpp ! -name *main.cpp)
TRACKER_MAIN := $(shell find $(TRACKER_DIR) -name *main.cpp)
TRACKER_ALL_SRCS := $(TRACKER_MAIN) $(TRACKER_SRCS)
TRACKER_ALL_OBJS := $(TRACKER_ALL_SRCS:%=$(BUILD_DIR)/%.o)
TRACKER_OBJS := $(TRACKER_SRCS:%=$(BUILD_DIR)/%.o)

PEER_SRCS := $(shell find $(PEER_DIR) -name *.cpp ! -name *main.cpp)
PEER_MAIN := $(shell find $(PEER_DIR) -name *main.cpp)
PEER_ALL_SRCS := $(PEER_MAIN) $(PEER_SRCS)
PEER_ALL_OBJS := $(PEER_ALL_SRCS:%=$(BUILD_DIR)/%.o)
PEER_OBJS := $(PEER_SRCS:%=$(BUILD_DIR)/%.o)

TEST_SRCS := $(shell find $(TEST_DIR) -name *.cpp)
TEST_OBJS := $(TEST_SRCS:%=$(BUILD_DIR)/%.o)

CXXFLAGS := -std=c++11 -g -Wall -I$(INC_DIR)
CXX := g++

all: tracker peer test

tracker: $(TRACKER_ALL_OBJS) $(OBJS)
	$(CXX) $(TRACKER_ALL_OBJS) $(OBJS) -o $@ -pthread

peer: $(PEER_ALL_OBJS) $(OBJS)
	$(CXX) $(PEER_ALL_OBJS) $(OBJS) -o $@ -pthread

test: $(TEST_OBJS) $(PEER_OBJS) $(TRACKER_OBJS) $(OBJS)
	$(CXX) $(TEST_OBJS) $(PEER_OBJS) $(TRACKER_OBJS) $(OBJS) -o $@ -static -lboost_unit_test_framework -pthread

$(BUILD_DIR)/%.cpp.o: %.cpp $(DEPS)
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean
clean:
	$(RM) -r $(BUILD_DIR)
	$(RM) tracker peer test

MKDIR_P ?= mkdir -p

