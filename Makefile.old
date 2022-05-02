# Credit: https://spin.atomicobject.com/2016/08/26/makefile-c-projects/
BUILD_DIR ?= ./build

SRCS := errors.cpp crc32.cpp
SRCS_tracker := $(SRCS) tracker-main.cpp tracker.cpp
OBJS_tracker := $(SRCS_tracker:%=$(BUILD_DIR)/%.o)
SRCS_peer:= $(SRCS) peer-main.cpp peer.cpp
OBJS_peer := $(SRCS_peer:%=$(BUILD_DIR)/%.o)

CXXFLAGS := -std=c++11 -g -Wall
CXX := g++

all: tracker peer

tracker: $(OBJS_tracker)
	$(CXX) $(OBJS_tracker) -o $@ -pthread

peer: $(OBJS_peer)
	$(CXX) $(OBJS_peer) -o $@ -pthread

$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean
clean:
	$(RM) -r $(BUILD_DIR)
	$(RM) tracker peer

MKDIR_P ?= mkdir -p

