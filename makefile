
TARGET := server
SOURCE_DIR   := src
INCLUDE_DIR  := include
OBJECT_DIR   := obj
BIN_DIR      := bin

CC := g++
CCFLAGS := -g -Wall -std=c++11

SRCFILES     := $(wildcard $(SOURCE_DIR)/*.cpp)
INCLUDES     := $(INCLUDE_DIR)
OBJECTS      := $(SRCFILES:$(SOURCE_DIR)/%.cpp=$(OBJECT_DIR)/%.o)

all: $(BIN_DIR)/$(TARGET)

$(BIN_DIR)/$(TARGET): $(OBJECTS)
	$(CC) $(CCFLAGS) -o $@ $(OBJECTS)

$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	$(CC) $(CCFLAGS) -I$(INCLUDES) -o $@ -c $<

.PHONT:clean
clean:
	rm $(BIN_DIR)/$(TARGET) $(OBJECTS)