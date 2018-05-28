# Makefile
BUILD_DIR := build
SRC_DIR := source
TEST_DIR:= test
SRC_EXT := cpp

CC = g++
CFLAGS = -std=c++14 -Werror 
DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CFLAGS +=-DDEBUG -g
endif

LDFLAGS =
LAST_MODIFIED_CXX_FILE = $(shell ls -rt $(SRC_DIR)/*\.cpp && ls -rt $(TEST_DIR)/*\.cpp  | tail -1)


SOURCES := $(shell find $(SRC_DIR) -type f -name *.$(SRC_EXT))
OBJECTS := $(subst $(SRC_DIR),$(BUILD_DIR),$(SOURCES:.$(SRC_EXT)=.o))

TARGET:= simulator

TEST_TARGET := ticker_test events_test  elevator_test

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	@echo "$(CC) $(CFLAGS)  -c -o $@ $<"; $(CC) $(CFLAGS)  -c -o $@ $<

$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	@echo "$(CC) $(CFLAGS) $^ -o $(BUILD_DIR)/$(TARGET) "; $(CC) $(CFLAGS_DEBUG)  $^ -o $(BUILD_DIR)/$(TARGET)

Test:$(TEST_TARGET)

ticker_test: $(BUILD_DIR)/ticker_test
$(BUILD_DIR)/ticker_test: $(SRC_DIR)/ticker.cpp  $(TEST_DIR)/ticker_test.cpp
	$(CC) $(CFLAGS) -o $@ $^

events_test:$(BUILD_DIR)/events_test
$(BUILD_DIR)/events_test: $(SRC_DIR)/events.cpp  $(SRC_DIR)/ticker.cpp  $(TEST_DIR)/events_test.cpp
	$(CC) $(CFLAGS) -o $@ $^

elevator_test: $(BUILD_DIR)/elevator_test
$(BUILD_DIR)/elevator_test: $(SRC_DIR)/elevator.cpp $(SRC_DIR)/events.cpp  $(SRC_DIR)/ticker.cpp  $(TEST_DIR)/elevator_test.cpp
	$(CC) $(CFLAGS) -o $@ $^

	
.PHONY: all

clean:
	@echo "$(RM) -r $(BUILD_DIR)/*"; $(RM) -r $(BUILD_DIR)/*

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)




