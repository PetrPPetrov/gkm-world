// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include "main.h"
#include "user_interface.h"

void drawUserInterface()
{
    imguiBeginFrame(
        g_current_mouse_x,
        g_current_mouse_y,
        (g_left_mouse_pressed ? IMGUI_MBUT_LEFT : 0) | (g_right_mouse_pressed ? IMGUI_MBUT_RIGHT : 0),
        g_current_mouse_z,
        g_window_width,
        g_window_height,
        g_char_pressed
    );

    if (g_main_menu_open)
    {
        //const float window_width = 300.0f;
        //const float window_height = 240.0f;
        //ImGui::SetNextWindowSize(ImVec2(window_width, window_height), ImGuiCond_Once);
        //ImGui::SetNextWindowPos(ImVec2((g_window_width - window_width) / 2.0f, (g_window_height - window_height) / 2.0f), ImGuiCond_Once);

        //ImGui::Begin("Gkm-World Menu", &g_main_menu_open/*, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove*/);
        ImGui::Begin("123", &g_main_menu_open/*, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove*/);
        //ImGui::Button("Options");
        //ImGui::Button("Exit");
        ImGui::End();
    }

    imguiEndFrame();
}
