#pragma once

#include <string>

namespace neptune {

std::string getFileFromPicker();

int popUpWindow(const char* title, const char* message);

void fixFileExplorerPolicy();

void debugFunction_01();

}