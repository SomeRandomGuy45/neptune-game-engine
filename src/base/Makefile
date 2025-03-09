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
LIBS_LINUX = -lGL -lGLEW -lglfw -lassimp
LIBS_MACOS = -framework OpenGL -lGLEW -lglfw -lassimp
LIBS_WINDOWS = -lopengl32 -lglew32 -lglfw3 -lassimp

# Platform-specific settings
ifeq ($(UNAME_S), Linux)
    LIBS = $(LIBS_LINUX)
    EXE = $(TARGET).so
    EXE2 = $(TARGET2)
    CXXFLAGS += -Wl,-rpath,'$$ORIGIN'
else ifeq ($(UNAME_S), Darwin)  # macOS
    LIBS = $(LIBS_MACOS)
    EXE = $(TARGET).dylib
    EXE2 = $(TARGET2)
else  # Windows
    LIBS = $(LIBS_WINDOWS)
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
