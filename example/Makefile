##
## Configuration
##

TARGET = example
TEST = official_only.nes

MOS6502 = ../MOS6502

SOURCES += $(MOS6502)/MOS6502.cpp
SOURCES += cpu.cpp
SOURCES += main.cpp

CXXFLAGS = -std=c++11 -I$(MOS6502)

##
## Rules
##

OBJECTS = $(SOURCES:%.cpp=%.o)

default: all

all: $(TARGET)

run: $(TARGET)
	./$(TARGET) $(TEST)

clean:
	rm -f $(TARGET) $(OBJECTS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)
