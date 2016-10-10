// DisWndTech.cpp --- Disabled Window Technique for Windows
// by Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// This file is public domain software (PDS).
#include <windows.h>
#include <stdio.h>

//#define ENABLED

static const TCHAR s_szName[] = TEXT("DisWndTech");

HINSTANCE   g_hInstance;
HWND        g_hMainWnd;
TCHAR       g_sz[64] = TEXT("");
BOOL        g_bVirtuallyActivated = FALSE;

void OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hDC = BeginPaint(hwnd, &ps);
    if (hDC == NULL)
        return;

    RECT rc;
    GetClientRect(hwnd, &rc);

    DrawText(hDC, g_sz, -1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

    EndPaint(hwnd, &ps);
}

void OnMouseMove(HWND hwnd, POINT ptScreen, BOOL bDown)
{
    static POINT ptPrev = {-1, -1};
    if (bDown && GetCapture() == hwnd)
    {
        if (ptPrev.x != -1 || ptPrev.y != -1)
        {
            RECT rc;
            GetWindowRect(hwnd, &rc);
            MoveWindow(hwnd,
                rc.left + (ptScreen.x - ptPrev.x),
                rc.top + (ptScreen.y - ptPrev.y),
                rc.right - rc.left,
                rc.bottom - rc.top,
                TRUE);
        }
        ptPrev = ptScreen;
    }
    else
    {
        ptPrev.x = -1;
        ptPrev.y = -1;
        ReleaseCapture();
    }
}

void OnLButton(HWND hwnd, POINT ptScreen, BOOL bDown)
{
    if (bDown)
    {
        SetCapture(hwnd);
    }
    else
    {
        ReleaseCapture();
    }
}

LRESULT CALLBACK
WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    POINT ptScreen;
    HWND hwndForeground;

    switch (uMsg)
    {
    case WM_CREATE:
#ifndef ENABLED
        // watch the key state
        SetTimer(hwnd, 999, 100, NULL);
#endif
        break;

    case WM_DESTROY:
#ifndef ENABLED
        KillTimer(hwnd, 999);
#endif
        PostQuitMessage(0);
        break;

#ifndef ENABLED
    case WM_TIMER:
        hwndForeground = GetForegroundWindow();
        if (hwndForeground != NULL && hwndForeground != hwnd)
        {
            g_bVirtuallyActivated = FALSE;
        }
        if (g_bVirtuallyActivated &&
            (GetKeyState('Q') < 0 || GetAsyncKeyState('Q') < 0))
        {
            DestroyWindow(hwnd);
        }
        break;
#endif

    case WM_ACTIVATE:
        // WM_ACTIVATE message doesn't come on disabled window
        if (LOWORD(wParam) == WA_INACTIVE)
        {
            g_bVirtuallyActivated = FALSE;
        }
        else
        {
            g_bVirtuallyActivated = TRUE;
        }
        break;

    case WM_KEYDOWN:
        lstrcpy(g_sz, TEXT("WM_KEYDOWN"));
        InvalidateRect(hwnd, NULL, TRUE);
        if (wParam == 'Q')
            DestroyWindow(hwnd);
        break;

    case WM_KEYUP:
        lstrcpy(g_sz, TEXT("WM_KEYUP"));
        InvalidateRect(hwnd, NULL, TRUE);
        break;

    case WM_SYSKEYDOWN:
        lstrcpy(g_sz, TEXT("WM_SYSKEYDOWN"));
        InvalidateRect(hwnd, NULL, TRUE);
        break;

    case WM_SYSKEYUP:
        lstrcpy(g_sz, TEXT("WM_SYSKEYUP"));
        InvalidateRect(hwnd, NULL, TRUE);
        break;

    case WM_MOUSEMOVE:
        // WM_MOUSEMOVE message doesn't come on disabled window
        lstrcpy(g_sz, TEXT("WM_MOUSEMOVE"));
        InvalidateRect(hwnd, NULL, TRUE);
        GetCursorPos(&ptScreen);
        OnMouseMove(hwnd, ptScreen, GetAsyncKeyState(VK_LBUTTON) < 0);
        break;

    case WM_LBUTTONDOWN:
        // WM_LBUTTONDOWN message doesn't come on disabled window
        lstrcpy(g_sz, TEXT("WM_LBUTTONDOWN"));
        InvalidateRect(hwnd, NULL, TRUE);
        GetCursorPos(&ptScreen);
        OnLButton(hwnd, ptScreen, TRUE);
        break;

    case WM_LBUTTONUP:
        // WM_LBUTTONUP message doesn't come on disabled window
        lstrcpy(g_sz, TEXT("WM_LBUTTONUP"));
        InvalidateRect(hwnd, NULL, TRUE);
        GetCursorPos(&ptScreen);
        OnLButton(hwnd, ptScreen, FALSE);
        break;

    case WM_PAINT:
        OnPaint(hwnd);
        break;

#ifndef ENABLED
    case WM_SETCURSOR:
        // WM_SETCURSOR message comes even on disabled window
        GetCursorPos(&ptScreen);
        switch (HIWORD(lParam))
        {
        case WM_MOUSEMOVE:
            OnMouseMove(hwnd, ptScreen, GetAsyncKeyState(VK_LBUTTON) < 0);
            break;
        case WM_LBUTTONDOWN:
            OnLButton(hwnd, ptScreen, TRUE);
            g_bVirtuallyActivated = TRUE;
            break;
        case WM_LBUTTONUP:
            OnLButton(hwnd, ptScreen, FALSE);
            break;
        }
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        break;
#endif

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

INT WINAPI WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       lpCmdLine,
    INT         nCmdShow)
{
    g_hInstance = hInstance;

    WNDCLASS wc;
    wc.style            = CS_DBLCLKS;
    wc.lpfnWndProc      = WindowProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInstance;
    wc.hIcon            = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_3DFACE + 1);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = s_szName;
    if (!RegisterClass(&wc))
    {
        MessageBoxA(NULL, "RegisterClass", NULL, MB_ICONERROR);
        return 1;
    }

#ifdef ENABLED
    DWORD dwStyle = WS_POPUP | WS_BORDER;
#else
    DWORD dwStyle = WS_POPUP | WS_BORDER | WS_DISABLED;
#endif
    DWORD dwExStyle = WS_EX_TOPMOST | WS_EX_DLGMODALFRAME | WS_EX_TOOLWINDOW;
    g_hMainWnd = CreateWindowEx(
        dwExStyle,
        s_szName,
        s_szName,
        dwStyle,
        0, 0, 250, 150,
        NULL,
        NULL,
        hInstance,
        NULL);
    if (g_hMainWnd == NULL)
    {
        MessageBoxA(NULL, "CreateWindow", NULL, MB_ICONERROR);
        return 2;
    }

    ShowWindow(g_hMainWnd, nCmdShow);
    UpdateWindow(g_hMainWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (INT)msg.wParam;
}
