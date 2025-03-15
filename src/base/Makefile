# Define the source and output
TARGET = neptuneSource
TARGET2 = test
SRC := $(wildcard *.cpp *.c)

# Platform detection
UNAME_S := $(shell uname -s)

# Compiler and standard
CXX = g++
CXXFLAGS = --std=c++20 -lm

# Libraries and frameworks
LIBS = $(shell sdl2-config --cflags --libs)

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
	$(CXX) $(CXXFLAGS) -o $(EXE) $(SRC) $(LIBS)

test:
	$(CXX) $(CXXFLAGS) -o $(EXE2) $(SRC) $(LIBS)

# Clean up the build
clean:
	rm -f $(TARGET) $(EXE)
	rm -f $(TARGET2) $(EXE2)