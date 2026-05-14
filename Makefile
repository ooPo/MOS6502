##
## MOS6502 CPU Emulator - Build Configuration
## by Naomi Peori (naomi@peori.ca)
##

CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -O2 \
           -Wno-gnu-anonymous-struct \
           -Wno-gnu-case-range \
           -Wno-unused-parameter

MOS6502_DIR     = MOS6502
NES_EXAMPLE_DIR = examples/nes
BUILD_DIR       = build

INCLUDES = -I$(MOS6502_DIR)

SOURCES = $(wildcard $(MOS6502_DIR)/*.cpp) \
          $(wildcard $(NES_EXAMPLE_DIR)/*.cpp)

OBJECTS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
DEPS    = $(OBJECTS:.o=.d)

TARGET       = mos6502_example
DEBUG_TARGET = mos6502_debug

TEST_ROM = $(NES_EXAMPLE_DIR)/official_only.nes

# ---------------------------------------------------------------------------

.PHONY: all debug clean run test help

all: $(TARGET)

debug: CXXFLAGS += -g -O0 -DDEBUG
debug: $(DEBUG_TARGET)

run: $(TARGET)
	@test -f $(TEST_ROM) || (echo "ERROR: Test ROM not found at $(TEST_ROM)" && exit 1)
	./$(TARGET) $(TEST_ROM)

test: run

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(DEBUG_TARGET)

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

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@

$(DEBUG_TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@

-include $(DEPS)
