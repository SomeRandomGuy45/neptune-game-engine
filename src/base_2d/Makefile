# Define the source and output
TARGET = libneptuneSource
TARGET2 = test
SRC := $(wildcard *.c *.cc)
SRC2 := $(wildcard *.cpp)

# Platform detection
UNAME_S := $(shell uname -s)

# Compiler and standard
CXX = g++
CXXFLAGS = --std=c++20 -lm -Wall
CXXFLAGS_APP = --std=c++20 -lm -Wall

# Libraries and frameworks
# Wont you want some ttf font files?
LIBS = $(shell pkg-config --cflags --libs SDL2_image SDL2_ttf sdl2 sdl2_mixer libzip) -llua

# Platform-specific settings
ifeq ($(UNAME_S), Linux)
    EXE = $(TARGET).so
    EXE2 = $(TARGET2)
    CXXFLAGS += -Wl,-rpath,'$$ORIGIN'
    CXXFLAGS_APP += -Wl,-rpath,@loader_path
else ifeq ($(UNAME_S), Darwin)
    EXE = $(TARGET).dylib
    EXE2 = $(TARGET2)
    CXXFLAGS += -dynamiclib -install_name @rpath/$(TARGET).dylib
    CXXFLAGS_APP += -Wl,-rpath,@loader_path
else
    EXE = $(TARGET).dll
    EXE2 = $(TARGET2).exe
    CXXFLAGS += -DGLFW_INCLUDE_NONE
endif

# Build target
main:
	$(CXX) $(CXXFLAGS) -shared -o $(EXE) $(SRC) $(LIBS)

test:
	$(CXX) $(CXXFLAGS_APP) -o $(TARGET2) $(SRC2) $(LIBS) -I. -L. -lneptuneSource
	

# Clean up the build
clean:
	rm -f $(TARGET) $(EXE)
	rm -f $(TARGET2) $(EXE2)