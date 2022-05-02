# Credit: https://spin.atomicobject.com/2016/08/26/makefile-c-projects/
SRC_DIR ?= ./src
TRACKER_DIR ?= $(SRC_DIR)/tracker
PEER_DIR ?= $(SRC_DIR)/peer
BUILD_DIR ?= ./bin
INC_DIR ?= ./include

SRCS := $(shell ls $(SRC_DIR)/*.cpp)
DEPS := $(shell find $(INC_DIR) -name *.h)
TRACKER_SRCS := $(SRCS) $(shell find $(TRACKER_DIR) -name *.cpp)
TRACKER_OBJS := $(TRACKER_SRCS:%=$(BUILD_DIR)/%.o)
PEER_SRCS := $(SRCS) $(shell find $(PEER_DIR) -name *.cpp)
PEER_OBJS := $(PEER_SRCS:%=$(BUILD_DIR)/%.o)
OBJS := $(TRACKER_OBJS) $(PEER_OBJS)

CXXFLAGS := -std=c++11 -g -Wall -I$(INC_DIR)
CXX := g++

all: tracker peer

tracker: $(TRACKER_OBJS)
	$(CXX) $(TRACKER_OBJS) -o $@ -pthread

peer: $(PEER_OBJS)
	$(CXX) $(PEER_OBJS) -o $@ -pthread

$(BUILD_DIR)/%.cpp.o: %.cpp $(DEPS)
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean
clean:
	$(RM) -r $(BUILD_DIR)
	$(RM) tracker peer

MKDIR_P ?= mkdir -p

