#include "helper.h"

#include <fstream>
#include <filesystem>

namespace neptune {

void game_log(const std::string& message, neptune::LogLevel level) {
    std::ofstream log_file(neptune::currentLogPath);

    if (!log_file.is_open()) {
        std::cerr << "Failed to open log file: " << neptune::currentLogPath << std::endl;
        exit(1);
    }    

    switch (level) {
        case neptune::DEBUG:
            std::cout << BOLD(FYEL("[DEBUG] ")) << message << std::endl;
            break;
        case neptune::WARNING:
            std::cout << BOLD(FYEL("[WARNING] ")) << message << std::endl;
            break;
        case neptune::ERROR:
            std::cerr << BOLD(FRED("[ERROR] ")) << message << std::endl;
            break;
        case neptune::CRITICAL:
            std::cerr <<  BOLD(FRED("[CRITICAL] ")) << message << std::endl;
            exit(1); // exit the game if critical error
        default:
            std::cout << BOLD(FWHT("[INFO] ")) << message << std::endl;
    }

    log_file.close();
}

std::string getExecutablePath() {
    char buffer[PATH_MAX];

    #ifdef _WIN32
        if (GetModuleFileName(NULL, buffer, MAX_PATH)) {
            return std::string(buffer);
        }
    #elif __APPLE__ // macOS-specific method
        uint32_t size = sizeof(buffer);
        if (_NSGetExecutablePath(buffer, &size) == 0) {
            return std::string(buffer);
        }
    #else // Linux
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (len != -1) {
            buffer[len] = '\0';
            return std::string(buffer);
        }
    #endif

    return ""; // Return an empty string if all methods fail
}

std::string get_current_logfile() {
    // get current date and time
    time_t now = time(0);
    tm* current_tm = localtime(&now);

    std::string log_filename = "game_log_" + std::to_string(current_tm->tm_year + 1900) + "-" +
                              std::to_string(current_tm->tm_mon + 1) + "-" +
                              std::to_string(current_tm->tm_mday) + ".txt";

    std::string log_path = getExecutablePath();
    std::filesystem::path execDir = std::filesystem::path(log_path).parent_path();
    
    // Move up one directory (remove executable's directory)
    execDir = execDir.parent_path();
    
    // Create logs path
    log_path = execDir.string();
    if (!std::filesystem::exists(log_path + "/logs")) {
        std::filesystem::create_directories(log_path + "/logs");
    }
    log_path += "/logs/" + log_filename;
    return log_path;
}

}