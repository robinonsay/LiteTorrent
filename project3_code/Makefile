# Credit: https://spin.atomicobject.com/2016/08/26/makefile-c-projects/
BUILD_DIR ?= ./build

SRCS := errors.cpp
SRCS_rSender := $(SRCS) rSender.cpp rUDPClient.cpp
OBJS_rSender := $(SRCS_rSender:%=$(BUILD_DIR)/%.o)
SRCS_rReceiver := $(SRCS) rReceiver.cpp rUDPServer.cpp
OBJS_rReceiver := $(SRCS_rReceiver:%=$(BUILD_DIR)/%.o)

CXXFLAGS := -g -Wall
CXX := g++

all: rSender rReceiver

rSender: $(OBJS_rSender)
	$(CXX) $(OBJS_rSender) -o $@

rReceiver: $(OBJS_rReceiver)
	$(CXX) $(OBJS_rReceiver) -o $@

$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean
clean:
	$(RM) -r $(BUILD_DIR)
	$(RM) rSender rReceiver

MKDIR_P ?= mkdir -p

