# Define the source and output
TARGET = neptuneSource
TARGET2 = test
SRC := $(wildcard *.c *.cc)
SRC2 := $(wildcard *.cpp *.c *.cc)

# Platform detection
UNAME_S := $(shell uname -s)

# Compiler and standard
CXX = g++
CXXFLAGS = --std=c++20 -lm

# Libraries and frameworks
# Wont you want some ttf font files?
LIBS = $(shell pkg-config --cflags --libs SDL2_image SDL2_ttf sdl2 sdl2_mixer) -llua

# Platform-specific settings
ifeq ($(UNAME_S), Linux)
    EXE = $(TARGET).so
    EXE2 = $(TARGET2)
    CXXFLAGS += -Wl,-rpath,'$$ORIGIN'
else ifeq ($(UNAME_S), Darwin)  # macOS
    EXE = $(TARGET).dylib
    EXE2 = $(TARGET2)
else  # Windows
    EXE = $(TARGET).dll
    EXE2 = $(TARGET2).exe
    CXXFLAGS += -DGLFW_INCLUDE_NONE  # In case you need to avoid including OpenGL headers with GLFW
endif

# Build target
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -shared -o $(EXE) $(SRC) $(LIBS)

test:
	$(CXX) $(CXXFLAGS) -o $(EXE2) $(SRC2) $(LIBS)

# Clean up the build
clean:
	rm -f $(TARGET) $(EXE)
	rm -f $(TARGET2) $(EXE2)