#include "imgui.h"
#include <iostream>
#include <sstream>;

#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <GL/GL.h>
#include <tchar.h>
#include "fin_calc.h"
// Data stored per platform window
struct WGL_WindowData { HDC hDC; };

// Data
static HGLRC            g_hRC;
static WGL_WindowData   g_MainWindow;
static int              g_Width;
static int              g_Height;

// Forward declarations of helper functions
bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data);
void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data);
void ResetDeviceWGL();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

enum Operation {
    PLUS = 0,
    MINUS = 1,
    MULT = 2,
    DIVIDE = 3
};

LongFloat PerformOp(const LongFloat& a, const LongFloat& b, int op) {
    switch (op) {
    case PLUS: {
        return a + b;
    }
    case MINUS: {
        return a - b;
    }
    case MULT: {
        return a * b;
    }
    case DIVIDE: {
        return a / b;
    }
    default: {
        throw std::invalid_argument("Something is wrong, please restart your computer");
    }
    }
}

// Main code
int main(int, char**)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_OWNDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"ImGui Example", NULL };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Financial calculator", WS_OVERLAPPEDWINDOW, 100, 100, 1200, 500, NULL, NULL, wc.hInstance, NULL);
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    // Initialize OpenGL
    if (!CreateDeviceWGL(hwnd, &g_MainWindow))
    {
        CleanupDeviceWGL(hwnd, &g_MainWindow);
        ::DestroyWindow(hwnd);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }
    wglMakeCurrent(g_MainWindow.hDC, g_hRC);

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\calibri.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_InitForOpenGL(hwnd);
    ImGui_ImplOpenGL3_Init();

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    static char first_num[64];
    static char second_num[64];
    static char third_num[64];
    static char fourth_num[64];

    static char result[64];
    strcpy(first_num, "0");
    strcpy(second_num, "0");
    strcpy(third_num, "0");
    strcpy(fourth_num, "0");
    // Main loop
    bool done = false;
    while (!done)
    {
        Sleep(1);
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

        // We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
        // Based on your use case you may want one or the other.
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);

        ImGui::Begin("Example: Fullscreen window", nullptr, flags);

        ImGui::Text(u8"Автор:");
        ImGui::Text(u8"Макаров Дмитрий Вадимович, группа 4, курс 4, год 2023");
        ImGui::SeparatorText(u8"Калькулятор");

        const char* items[] = { "+", "-", "*", "/" };
        static int first_op = 0;
        static int second_op = 0;
        static int third_op = 0;
        
        ImGui::SetNextItemWidth(200);
        ImGui::InputTextWithHint("##First number", u8"Число 1", first_num, 64);
        ImGui::SameLine();
        //num + (num + num) + num;
        ImGui::SetNextItemWidth(40);
        ImGui::Combo("##Operation1Combo", &first_op, items, IM_ARRAYSIZE(items));
        ImGui::SameLine();
        
        ImGui::Text("(");
        ImGui::SameLine();

        ImGui::SetNextItemWidth(200);
        ImGui::InputTextWithHint("##Second number", u8"Число 2", second_num, 64);
        ImGui::SameLine();

        ImGui::SetNextItemWidth(40);
        ImGui::Combo("##Operation2Combo", &second_op, items, IM_ARRAYSIZE(items));
        ImGui::SameLine();

        ImGui::SetNextItemWidth(200);
        ImGui::InputTextWithHint("##Third number", u8"Число 3", third_num, 64);
        ImGui::SameLine();

        ImGui::Text(")");
        ImGui::SameLine();

        ImGui::SetNextItemWidth(40);
        ImGui::Combo("##Operation3Combo", &third_op, items, IM_ARRAYSIZE(items));
        ImGui::SameLine();

        ImGui::SetNextItemWidth(200);
        ImGui::InputTextWithHint("##Fourth number", u8"Число 4", fourth_num, 64);
        
        ImGui::Text(u8"Результат:");
        ImGui::SameLine();
        try {
            LongFloat result_prior;
            LongFloat first_float{ std::string(first_num) };
            LongFloat second_float{ std::string(second_num) };
            LongFloat third_float{ std::string(third_num) };
            LongFloat fourth_float{ std::string(fourth_num) };
            LongFloat result_float;
            result_prior = PerformOp(second_float, third_float, (Operation)second_op);
            if ((first_op < 2 && second_op < 2) ||
                (first_op >= 2 && second_op >= 2) || 
                (first_op >= 2 && second_op < 2)) {
                result_float = PerformOp(PerformOp(first_float, result_prior, first_op), fourth_float, third_op);
            }
            if (first_op < 2 && second_op >= 2) {
                result_float = PerformOp(first_float, PerformOp(result_prior, fourth_float, third_op), first_op);
            }
            ImGui::Text(result_float.to_string().c_str());
            ImGui::SeparatorText(u8"Округление: (1 - математическое, 2 - банковское, 3 - усечение)");
            static int round_op = 0;
            static const char* round_options[] = {"1", "2", "3"};
            ImGui::Text(u8"Тип:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(40);
            ImGui::Combo("##OperationRoundCombo", &round_op, round_options, IM_ARRAYSIZE(round_options));
            std::stringstream ss;
            ImGui::SameLine();
            switch (round_op) {
            case 0: {
                LongFloat tmp = result_float;
                tmp.MathRound();
                ss << u8"Матем(" << result_float.to_string().c_str() << ") = " << tmp.to_string().c_str();
                ImGui::Text(ss.str().c_str());
                break;
            }
            case 1: {
                LongFloat tmp = result_float;
                tmp.BankRound();
                ss << u8"Банк(" << result_float.to_string().c_str() << ") = " << tmp.to_string().c_str();
                ImGui::Text(ss.str().c_str());
                break;
            }
            case 2: {
                LongFloat tmp = result_float;
                tmp.TruncRound();
                ss << u8"Усеч(" << result_float.to_string().c_str() << ") = " << tmp.to_string().c_str();
                ImGui::Text(ss.str().c_str());
                break;
            }
            }
        }
        catch (const std::invalid_argument& e) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), e.what());
        }
        ImGui::End();
      //  ImGui::ShowDemoWindow();


        // Rendering
        ImGui::Render();
        glViewport(0, 0, g_Width, g_Height);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Present
        ::SwapBuffers(g_MainWindow.hDC);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceWGL(hwnd, &g_MainWindow);
    wglDeleteContext(g_hRC);
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions
bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
    HDC hDc = ::GetDC(hWnd);
    PIXELFORMATDESCRIPTOR pfd = { 0 };
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    const int pf = ::ChoosePixelFormat(hDc, &pfd);
    if (pf == 0)
        return false;
    if (::SetPixelFormat(hDc, pf, &pfd) == FALSE)
        return false;
    ::ReleaseDC(hWnd, hDc);

    data->hDC = ::GetDC(hWnd);
    if (!g_hRC)
        g_hRC = wglCreateContext(data->hDC);
    return true;
}

void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
    wglMakeCurrent(NULL, NULL);
    ::ReleaseDC(hWnd, data->hDC);
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
        {
            g_Width = LOWORD(lParam);
            g_Height = HIWORD(lParam);
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}