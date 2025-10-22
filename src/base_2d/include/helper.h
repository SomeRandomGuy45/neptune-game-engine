#ifndef HELPER
#define HELPER
#include <iostream>
#include <string>
#include <fstream>

/*
* Lib stuff going to be RIPPED from simple
* Its going to work different
*/

#ifdef _WIN32
#include <windows.h>
#define LIB_EXT ".dll"
#define LIB_LOAD(name) LoadLibraryA(name)
#define LIB_UNLOAD(lib) FreeLibrary((HMODULE)lib)
#define LIB_GETFUNC(lib, func_name) reinterpret_cast<void*>(GetProcAddress((HMODULE)lib, func_name))
#define OS "WIN"
#else
#include <unistd.h>
#include <limits.h>
#include <dlfcn.h>
#define LIB_LOAD(name) dlopen(name, RTLD_LAZY)
#define LIB_UNLOAD(lib) dlclose(lib)
#define LIB_GETFUNC(lib, func_name) dlsym(lib, func_name)
#endif

#ifdef __APPLE__
#include <mach-o/dyld.h>
#define LIB_EXT ".dylib"
#define OS "MACOS"
#elif linux
#define LIB_EXT ".so"
#define OS "LINUX"
#endif

namespace neptune {

enum NEPTUNE_LOGLEVEL {
    DEFAULT,
    DEBUG,
    WARNING,
    ERROR,
    CRITICAL // last thing the game would see... if critical we just kill it...
};

/* FOREGROUND */
#define RST  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST

#define BOLD(x) "\x1B[1m" x RST
#define UNDL(x) "\x1B[4m" x RST

void game_log(const std::string& message, NEPTUNE_LOGLEVEL level = DEFAULT);

std::string getExecutablePath();

std::string get_current_logfile();

inline std::string currentLogPath = get_current_logfile();

}
#endif