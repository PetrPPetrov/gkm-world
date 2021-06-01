// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <iostream>
#include <boost/asio/impl/src.hpp>
#include "bgfx_engine.h"
#include "log.h"
#include "user_interface.h"
#include "main.h"

bool g_is_running = true;
HINSTANCE g_hinstance = 0;
HWND g_hwnd = 0;
BgfxEngine::Ptr g_bgfx_engine = nullptr;

int g_screen_width = 0;
int g_screen_height = 0;
int g_window_width = 800;
int g_window_height = 600;

int g_char_pressed = -1;
bool g_left_mouse_pressed = false;
bool g_right_mouse_pressed = false;
int g_current_mouse_x = 0;
int g_current_mouse_y = 0;
int g_current_mouse_z = 0;

PlayerLocation g_player_location;
KeyboardState g_keyboard_state;

bool g_main_menu_open = true;

std::string g_server_ip_address;
unsigned short g_server_port_number;
long g_ping = 0;

AnotherPlayer* g_other_players_display_list = nullptr;
OtherPlayersMap* g_uuid_to_another_user = nullptr;
std::uint32_t g_user_token = 0;
UDPConnection* g_connection = nullptr;
bool g_logout_request = false;

extern Log::Logger* g_logger = nullptr;

static void renderScene()
{
    bgfx::reset(g_window_width, g_window_height);
    drawUserInterface();
    g_bgfx_engine->draw();
    gameStep(g_player_location, g_keyboard_state);
}

static void onLeftMouseButtonDown(LPARAM l_param)
{
    g_current_mouse_x = GET_X_LPARAM(l_param);
    g_current_mouse_y = GET_Y_LPARAM(l_param);
    g_left_mouse_pressed = true;
}

static void onLeftMouseButtonUp(LPARAM l_param)
{
    g_current_mouse_x = GET_X_LPARAM(l_param);
    g_current_mouse_y = GET_Y_LPARAM(l_param);
    g_left_mouse_pressed = false;
}

static void onRightMouseButtonDown(LPARAM l_param)
{
    g_current_mouse_x = GET_X_LPARAM(l_param);
    g_current_mouse_y = GET_Y_LPARAM(l_param);
    g_right_mouse_pressed = true;
}

static void onRightMouseButtonUp(LPARAM l_param)
{
    g_current_mouse_x = GET_X_LPARAM(l_param);
    g_current_mouse_y = GET_Y_LPARAM(l_param);
    g_right_mouse_pressed = false;
}

static void onMouseMove(LPARAM l_param)
{
    g_current_mouse_x = GET_X_LPARAM(l_param);
    g_current_mouse_y = GET_Y_LPARAM(l_param);
}

static void onSizing(RECT* rect)
{
    g_window_width = rect->right - rect->left;
    g_window_height = rect->bottom - rect->top;
}

static void onSize(LPARAM l_param)
{
    g_window_width = GET_X_LPARAM(l_param);
    g_window_height = GET_Y_LPARAM(l_param);
}

static LRESULT APIENTRY CALLBACK onWindowEvent(HWND window_handle, UINT window_msg, WPARAM w_param, LPARAM l_param)
{
    LRESULT result = 1;
    switch (window_msg)
    {
    case WM_LBUTTONDOWN:
        onLeftMouseButtonDown(l_param);
        break;
    case WM_LBUTTONUP:
        onLeftMouseButtonUp(l_param);
        break;
    case WM_RBUTTONDOWN:
        onRightMouseButtonDown(l_param);
        break;
    case WM_RBUTTONUP:
        onRightMouseButtonUp(l_param);
        break;
    case WM_MOUSEMOVE:
        onMouseMove(l_param);
        break;
    case WM_SIZING:
        onSizing(reinterpret_cast<RECT*>(l_param));
        break;
    case WM_SIZE:
        onSize(l_param);
        break;
    case WM_KEYDOWN:
        switch (w_param)
        {
        case VK_ESCAPE:
            g_main_menu_open = !g_main_menu_open;
            break;
        case VK_UP:
        case 0x57:
            g_keyboard_state.up = true;
            break;
        case VK_DOWN:
        case 0x53:
            g_keyboard_state.down = true;
            break;
        case VK_LEFT:
        case 0x41:
            g_keyboard_state.left = true;
            break;
        case VK_RIGHT:
        case 0x44:
            g_keyboard_state.right = true;
            break;
        }
        break;
    case WM_KEYUP:
        switch (w_param)
        {
        case VK_UP:
        case 0x57:
            g_keyboard_state.up = false;
            break;
        case VK_DOWN:
        case 0x53:
            g_keyboard_state.down = false;
            break;
        case VK_LEFT:
        case 0x41:
            g_keyboard_state.left = false;
            break;
        case VK_RIGHT:
        case 0x44:
            g_keyboard_state.right = false;
            break;
        }
        break;
    case WM_CLOSE:
        g_is_running = false;
        break;
    default:
        result = DefWindowProc(window_handle, window_msg, w_param, l_param);
        break;
    }
    return result;
}

static void registerWindowClass()
{
    g_screen_width = GetSystemMetrics(SM_CXSCREEN);
    g_screen_height = GetSystemMetrics(SM_CYSCREEN);

    WNDCLASS window_class;
    window_class.style = CS_OWNDC;
    window_class.lpfnWndProc = (WNDPROC)onWindowEvent;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = g_hinstance;
    window_class.hIcon = LoadIcon(g_hinstance, IDI_APPLICATION);
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    window_class.lpszMenuName = NULL;
    window_class.lpszClassName = "Gkm-World";

    if (!RegisterClass(&window_class))
    {
        MessageBox(NULL, "Couldn't Register Window Class", "Error", MB_ICONERROR);
        exit(1);
    }
}

static HWND createWindow()
{
    g_window_width = g_screen_width * 3 / 4;
    g_window_height = g_screen_height * 3 / 4;

    int window_coord_x = (g_screen_width - g_window_width) / 2;
    int window_coord_y = (g_screen_height - g_window_height) / 2;

    HWND window = CreateWindowEx(WS_EX_APPWINDOW, "Gkm-World", "Gkm-World", WS_OVERLAPPEDWINDOW, window_coord_x, window_coord_y, g_window_width, g_window_height, NULL, NULL, g_hinstance, NULL);
    MoveWindow(window, window_coord_x, window_coord_y, g_window_width, g_window_height, FALSE);
    return window;
}

static void initializeBGFX(bgfx::RendererType::Enum render_type)
{
    g_bgfx_engine = std::make_unique<BgfxEngine>(g_window_width, g_window_height, g_hwnd, render_type);
    imguiCreate();
}

static void shutdownBGFX()
{
    imguiDestroy();
    g_bgfx_engine = nullptr;
}

static int winMainImpl(HINSTANCE h_instance, HINSTANCE h_prev_instance, LPSTR lp_cmd_line, INT n_cmd_show)
{
    Log::Holder log_holder;
    g_logger = new Log::Logger(Packet::ESeverityType::DebugMessage, "win_client.log", false, true);
    LOG_INFO << "Node Server is starting...";

    g_hinstance = h_instance;
    registerWindowClass();
    g_hwnd = createWindow();
    if (!g_hwnd)
    {
        MessageBox(NULL, "Couldn't Create Window", "Error", MB_ICONERROR);
        exit(1);
    }
    ShowWindow(g_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(g_hwnd);
    SetFocus(g_hwnd);
    initializeBGFX(bgfx::RendererType::Direct3D11);

    MSG window_event;
    while (g_is_running)
    {
        renderScene();

        while (PeekMessage(&window_event, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&window_event);
            DispatchMessage(&window_event);
        }
    }

    shutdownBGFX();
    DestroyWindow(g_hwnd);
    return static_cast<int>(window_event.wParam);
}

int WINAPI WinMain(HINSTANCE h_instance, HINSTANCE h_prev_instance, LPSTR lp_cmd_line, INT n_cmd_show)
{
    try
    {
        return winMainImpl(h_instance, h_prev_instance, lp_cmd_line, n_cmd_show);
    }
    catch (const std::exception& exception)
    {
        MessageBox(0, exception.what(), "std::exception was thrown", MB_OK);
        std::cout << "std::exception was thrown: " << exception.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        MessageBox(0, "description is not available", "unknown exception was thrown", MB_OK);
        std::cout << "unknown exception was thrown" << std::endl;
        return EXIT_FAILURE;
    }
}
