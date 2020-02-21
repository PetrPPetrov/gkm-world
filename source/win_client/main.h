// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <unordered_map>
#include "game_logic.h"
#include "bgfx_engine.h"
#include "bgfx_windows.h"
#include "another_player.h"

typedef std::unordered_map<std::uint32_t, AnotherPlayer::Ptr> OtherPlayersMap;
class UDPConnection;

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

extern std::string g_server_ip_address;
extern unsigned short g_server_port_number;
extern long g_ping;

extern AnotherPlayer* g_other_players_display_list;
extern OtherPlayersMap* g_uuid_to_another_user;
extern std::uint32_t g_user_token;
extern UDPConnection* g_connection;
extern UDPConnection::EState g_main_state;
extern bool g_logout_request;
