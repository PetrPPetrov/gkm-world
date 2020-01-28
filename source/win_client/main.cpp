// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <iostream>
#include "bgfx_engine.h"
#include "main.h"

extern bool g_is_running = true;
extern HINSTANCE g_hinstance = 0;
extern HWND g_hwnd = 0;
extern BgfxEngine::Ptr g_bgfx_engine = nullptr;

extern int g_screen_width = 0;
extern int g_screen_height = 0;
extern int g_window_width = 800;
extern int g_window_height = 600;

extern int g_char_pressed = -1;
extern bool g_left_mouse_pressed = false;
extern bool g_right_mouse_pressed = false;
extern int g_current_mouse_x = 0;
extern int g_current_mouse_y = 0;
extern int g_current_mouse_z = 0;

static double g_near = 8.0;
static double g_far = 16 * 1024.0;

static void renderScene()
{
    g_bgfx_engine->draw();
}

static LRESULT APIENTRY CALLBACK onWindowEvent(HWND window_handle, UINT window_msg, WPARAM w_param, LPARAM l_param)
{
    LRESULT result = 1;
    switch (window_msg)
    {
    case WM_KEYDOWN:
        switch (w_param)
        {
        case VK_ESCAPE:
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

    g_window_width = g_screen_width * 3 / 4;
    g_window_height = g_screen_height * 3 / 4;

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
