#include <windows.h>
#include <gdiplus.h>
#include "tinyxml2.h"
using namespace Gdiplus;

#pragma comment (lib, "Gdiplus.lib")

// Globals
float g_zoom = 1.0f;
float g_angle = 0.0f;

// Draw something
VOID OnPaint(HDC hdc, RECT rc)
{
    Graphics g(hdc);
    g.SetSmoothingMode(SmoothingModeHighQuality);

    // Get center of window
    int cx = (rc.right - rc.left) / 2;
    int cy = (rc.bottom - rc.top) / 2;

    // Apply transforms
    g.TranslateTransform(cx, cy);
    g.RotateTransform(g_angle);
    g.ScaleTransform(g_zoom, g_zoom);
    g.TranslateTransform(-cx, -cy);

    // Draw axes
    Pen axisPen(Color(255, 200, 200, 200), 1);
    g.DrawLine(&axisPen, cx, 0, cx, rc.bottom);
    g.DrawLine(&axisPen, 0, cy, rc.right, cy);

    // Draw rectangle
    Pen pen(Color(255, 0, 0, 255), 4);
    SolidBrush brush(Color(100, 0, 0, 255));
    Rect rect(cx - 100, cy - 50, 200, 100);
    g.FillRectangle(&brush, rect);
    g.DrawRectangle(&pen, rect);
}

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT iCmdShow)
{
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    WNDCLASS wc = { 0 };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = TEXT("GDIPlusZoomRotate");
    RegisterClass(&wc);

    HWND hWnd = CreateWindow(
        wc.lpszClassName, TEXT("GDI+ Zoom & Rotate Demo"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600, NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, iCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(gdiplusToken);
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static RECT clientRect;

    switch (msg)
    {
    case WM_CREATE:
        GetClientRect(hWnd, &clientRect);
        return 0;

    case WM_MOUSEWHEEL:
        if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
            g_zoom *= 1.1f;
        else
            g_zoom *= 0.9f;
        InvalidateRect(hWnd, NULL, TRUE);
        return 0;

    case WM_KEYDOWN:
        if (wParam == VK_LEFT)
            g_angle -= 5.0f;
        else if (wParam == VK_RIGHT)
            g_angle += 5.0f;
        InvalidateRect(hWnd, NULL, TRUE);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        GetClientRect(hWnd, &clientRect);
        OnPaint(hdc, clientRect);
        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}
