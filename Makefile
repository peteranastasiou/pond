
.DEFAULT_GOAL := all

CC = g++

# Whether to build for debugging instead of release
DEBUG = 0

# Whether to enable verbose execution trace debugging
DEBUG_TRACE_EXECUTION = 1

# Compilation flags
CFLAGS = -std=c++17 -W -Wall -Wextra -Werror -Wno-unused -Wconversion -MMD -MP -fno-exceptions
ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -O0 -g
else
	CFLAGS += -DNDEBUG -O2 -flto
endif

# Linker flags
LDFLAGS = 
ifeq ($(DEBUG), 1)
	LDFLAGS += 
else
	LDFLAGS += -O2
endif

TARGET = bin/pond

OBJECTS = $(patsubst src/%.cpp, build/%.o, $(wildcard src/*.cpp))
DEPS = $(OBJECTS:.o=.d)

ifeq ($(OS), Windows_NT)
	MKDIR_BUILD = if not exist build md build
	MKDIR_BIN = if not exist bin md bin
	RMDIR = rd /s /q
else
	MKDIR_BUILD = mkdir -p build
	MKDIR_BIN = mkdir -p bin
	RMDIR = rm -rf
endif

# Defines
DEFINES = 
ifeq ($(DEBUG_TRACE_EXECUTION), 1)
	DEFINES += -DDEBUG_TRACE_EXECUTION
endif

all: $(TARGET)

# Link
$(TARGET): $(OBJECTS)
	@$(MKDIR_BIN)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

# Compile sources
build/%.o: src/%.cpp
	@$(MKDIR_BUILD)
	$(CC) $(CFLAGS) $(DEFINES) -c $< -o $@

.PHONY: clean

clean:
	$(RMDIR) build
	$(RMDIR) bin

-include $(DEPS)
