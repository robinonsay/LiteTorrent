# Credit: https://spin.atomicobject.com/2016/08/26/makefile-c-projects/
SRC_DIR ?= ./src
HUB_DIR ?= $(SRC_DIR)/hub
PEER_DIR ?= $(SRC_DIR)/peer
TEST_DIR ?= $(SRC_DIR)/tests
MUTEX_DIR ?= $(SRC_DIR)/mutex
BUILD_DIR ?= ./bin
INC_DIR ?= ./include

SRCS := $(shell ls $(SRC_DIR)/*.cpp) $(shell find $(MUTEX_DIR) -name *.cpp)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o) 
DEPS := $(shell find $(INC_DIR) -name *.h)

HUB_SRCS := $(shell find $(HUB_DIR) -name *.cpp ! -name *main.cpp)
HUB_MAIN := $(shell find $(HUB_DIR) -name *main.cpp)
HUB_ALL_SRCS := $(HUB_MAIN) $(HUB_SRCS)
HUB_ALL_OBJS := $(HUB_ALL_SRCS:%=$(BUILD_DIR)/%.o)
HUB_OBJS := $(HUB_SRCS:%=$(BUILD_DIR)/%.o)

PEER_SRCS := $(shell find $(PEER_DIR) -name *.cpp ! -name *main.cpp)
PEER_MAIN := $(shell find $(PEER_DIR) -name *main.cpp)
PEER_ALL_SRCS := $(PEER_MAIN) $(PEER_SRCS)
PEER_ALL_OBJS := $(PEER_ALL_SRCS:%=$(BUILD_DIR)/%.o)
PEER_OBJS := $(PEER_SRCS:%=$(BUILD_DIR)/%.o)

TEST_SRCS := $(shell find $(TEST_DIR) -name *.cpp)
TEST_OBJS := $(TEST_SRCS:%=$(BUILD_DIR)/%.o)

CXXFLAGS := -std=c++11 -g -Wall -I$(INC_DIR)
CXX := g++

all: peer hub test

hub: $(HUB_ALL_OBJS) $(OBJS)
	$(CXX) $(HUB_ALL_OBJS) $(OBJS) -o $@ -pthread

peer: $(PEER_ALL_OBJS) $(OBJS)
	$(CXX) $(PEER_ALL_OBJS) $(OBJS) -o $@ -pthread

test: $(TEST_OBJS) $(PEER_OBJS) $(HUB_OBJS) $(OBJS)
	$(CXX) $(TEST_OBJS) $(PEER_OBJS) $(HUB_OBJS) $(OBJS) -o $@ -static -lboost_unit_test_framework -pthread

$(BUILD_DIR)/%.cpp.o: %.cpp $(DEPS)
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean
clean:
	$(RM) -r $(BUILD_DIR)
	$(RM) hub peer test

MKDIR_P ?= mkdir -p

