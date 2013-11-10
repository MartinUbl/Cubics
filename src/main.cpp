#include "global.h"
#include "app.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    if (!sApplication->Init())
        return -1;

    sApplication->Run();

    return 0;
}

Application::Application()
{
    m_windowWidth = 640;
    m_windowHeight = 600;

    stage = STAGE_BEGIN;
}

LRESULT CALLBACK MyWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_MOUSEMOVE)
    {
        sApplication->SetMouseXY(LOWORD(lParam), HIWORD(lParam));
        return 0;
    }

    return sSimplyFlat->SFWndProc(hWnd, msg, wParam, lParam);
}

void KeyPressed(uint16 key, bool pressed)
{
    sApplication->KeyEvent((uint8)key, pressed);
}

void MouseButtonPress(bool left, bool pressed)
{
    sApplication->MousePress(left, pressed);
}

bool Application::Init()
{
    if (!sSimplyFlat->CreateMainWindow("Cubics", m_windowWidth, m_windowHeight, 32, false, 60, &MyWndProc))
        return false;

    sSimplyFlat->Interface->HookEvent(0, KeyPressed);
    sSimplyFlat->Interface->HookMouseEvent(MouseButtonPress);

    for (uint32 i = 0; i < MAX_FONT; i++)
        m_fontMap[i] = -1;

    SetFontId(FONT_DEFAULT,      sSimplyFlat->BuildFont("Arial", 14));
    SetFontId(FONT_TITLE,        sSimplyFlat->BuildFont("Comic Sans MS", 26));
    /*SetFontId(FONT_MENU_ITEM,    sSimplyFlat->BuildFont("Times New Roman", 18));
    SetFontId(FONT_HINT,         sSimplyFlat->BuildFont("Times New Roman", 12));*/

    for (uint32 i = 0; i < MAX_FONT; i++)
    {
        if (m_fontMap[i] == -1)
            return false;
    }

    return true;
}

void Application::Run()
{
    MSG msg;

    while (true)
    {
        if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
        {
            if (msg.message != WM_QUIT)
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
                break;

            continue;
        }

        SF->BeforeDraw();

        RunGameplay();

        SF->AfterDraw();
    }
}
