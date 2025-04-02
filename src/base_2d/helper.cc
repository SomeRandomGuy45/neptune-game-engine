#include "helper.h"

#include <filesystem>
#include <ctime>

namespace neptune {

std::fstream log_file;

void open_log_file() {
    if (!std::filesystem::exists(neptune::currentLogPath)) {
        std::ofstream file(neptune::currentLogPath);
        file.close(); // Only create the file if it doesn't exist
    }

    log_file.open(neptune::currentLogPath, std::ios::app); // Open in append mode
    if (!log_file.is_open()) {
        std::cerr << "Failed to open log file: " << neptune::currentLogPath << std::endl;
        exit(1);
    }
}

void game_log(const std::string& message, neptune::NEPTUNE_LOGLEVEL level) {

    if (!log_file.is_open()) {
        open_log_file();
    }

    switch (level) {
        case neptune::DEBUG:
            std::cout << BOLD(FYEL("[DEBUG] ")) << message << std::endl;
            log_file << "[DEBUG] " << message << std::endl;
            break;
        case neptune::WARNING:
            std::cout << BOLD(FYEL("[WARNING] ")) << message << std::endl;
            log_file << "[WARNING] " << message << std::endl;
            break;
        case neptune::ERROR:
            std::cerr << BOLD(FRED("[ERROR] ")) << message << std::endl;
            log_file << "[ERROR] " << message << std::endl;
            break;
        case neptune::CRITICAL:
            std::cerr <<  BOLD(FRED("[CRITICAL] ")) << message << std::endl;
            log_file << "[CRITICAL] " << message << std::endl;
            log_file.close();
            exit(1); // exit the game if critical error
        default:
            std::cout << BOLD(FWHT("[INFO] ")) << message << std::endl;
            log_file << "[INFO] " << message << std::endl;
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
    std::time_t t = std::time(0);

    std::string log_filename = "game_log_" + std::to_string(t) + ".txt";

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