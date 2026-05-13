##
## MOS6502 CPU Emulator - Build Configuration
## by Naomi Peori (naomi@peori.ca)
##

# Compiler and standard.  Override CXX from the environment to use clang++, etc.
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -O2 \
           -Wno-gnu-anonymous-struct \
           -Wno-gnu-case-range \
           -Wno-unused-parameter

# Directories
MOS6502_DIR     = MOS6502
NES_EXAMPLE_DIR = examples/nes
BUILD_DIR       = build

# Include paths (library only; example includes library headers)
INCLUDES = -I$(MOS6502_DIR)

# Source files
SOURCES = $(wildcard $(MOS6502_DIR)/*.cpp) \
          $(wildcard $(NES_EXAMPLE_DIR)/*.cpp)

# Object and dependency files — kept in BUILD_DIR to avoid cluttering sources
OBJECTS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
DEPS    = $(OBJECTS:.o=.d)

# Targets
TARGET       = mos6502_example
DEBUG_TARGET = mos6502_debug

# Test ROM used by `make run` / `make test`
TEST_ROM = $(NES_EXAMPLE_DIR)/official_only.nes

# ---------------------------------------------------------------------------

.PHONY: all debug clean run test help

## Default target — build the release version
all: $(TARGET)

## Debug build — symbols enabled, optimisation off
debug: CXXFLAGS += -g -O0 -DDEBUG
debug: $(DEBUG_TARGET)

## Build and run with the test ROM
run: $(TARGET)
	@test -f $(TEST_ROM) || (echo "ERROR: Test ROM not found at $(TEST_ROM)" && exit 1)
	./$(TARGET) $(TEST_ROM)

## Alias for run
test: run

## Remove all build artefacts
clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(DEBUG_TARGET)

## Show this help
help:
	@echo "MOS6502 CPU Emulator — Makefile targets:"
	@echo ""
	@echo "  all (default)  Build the release binary ($(TARGET))"
	@echo "  debug          Build with debug symbols ($(DEBUG_TARGET))"
	@echo "  run            Build and run with the test ROM"
	@echo "  test           Alias for run"
	@echo "  clean          Remove all build artefacts"
	@echo "  help           Show this help"
	@echo ""
	@echo "  Override the compiler with: make CXX=clang++"

# ---------------------------------------------------------------------------
# Compilation rules
# ---------------------------------------------------------------------------

# -MMD -MP: generate .d dependency files alongside each .o so that editing
# a header causes the affected .cpp files to be recompiled automatically.
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

# Release link
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@

# Debug link
$(DEBUG_TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@

# Include generated dependency files (silently ignored on first build)
-include $(DEPS)
