// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include "logic.h"
#include "bgfx_windows.h"
#include "bgfx_engine.h"

extern bool g_is_running;
extern HINSTANCE g_hinstance;
extern HWND g_hwnd;
extern BgfxEngine::Ptr g_bgfx_engine;

extern int g_screen_width;
extern int g_screen_height;
extern int g_window_width;
extern int g_window_height;

extern int g_char_pressed;
extern bool g_left_mouse_pressed;
extern bool g_right_mouse_pressed;
extern int g_current_mouse_x;
extern int g_current_mouse_y;
extern int g_current_mouse_z;

extern PlayerLocation g_player_location;
extern KeyboardState g_keyboard_state;

extern bool g_main_menu_open;
