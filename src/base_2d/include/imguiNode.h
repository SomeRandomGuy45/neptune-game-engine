#pragma once
#include <iostream>
#include <string>
#include <list>
#include "imgui/imgui.h"

#include "helper.h"

class ImGuiNode {
public:
    // new___ = ImGui::Begin___
    bool newNode();
    bool newChild();
    bool newTooltip();
};