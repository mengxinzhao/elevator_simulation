#Run "make <program_name>" for release build and "make <program_name>_dbg" for debug build
BUILD_DIR := ./
SRC_DIR := ./

CXX_COMPILER := g++
CXX_FLAGS := -std=c++14  -Werror 
CXX_FLAGS_DEBUG := $(CXX_FLAGS) -DDEBUG 
CXX_FLAGS_RELEASE := $(CXX_FLAGS) -g  

LAST_MODIFIED_CXX_FILE = $(shell ls -rt $(SRC_DIR)/*\.cpp | tail -1)


last: $(notdir $(basename $(LAST_MODIFIED_CXX_FILE)))
last_dbg: $(notdir $(basename $(LAST_MODIFIED_CXX_FILE)))_dbg
clean:
	rm -r $(BUILD_DIR)
.PHONY: last last_dbg clean	all all

all : events_test ticker_test elevator_test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/events_test: $(SRC_DIR)/events_test.cpp $(BUILD_DIR)
	$(CXX_COMPILER) $(CXX_FLAGS_RELEASE) -o $@ $<

$(BUILD_DIR)/ticker_test: $(SRC_DIR)/ticker_test.cpp $(BUILD_DIR)
	$(CXX_COMPILER) $(CXX_FLAGS_RELEASE) -o $@ $<

$(BUILD_DIR)/elevator_test: $(SRC_DIR)/elevator_test.cpp $(BUILD_DIR)
	$(CXX_COMPILER) $(CXX_FLAGS_RELEASE) -o $@ $<
