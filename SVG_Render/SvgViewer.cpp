#include "SvgViewer.h"
#include "SvgDocument.h"
#include "Parser.h"
#include "FileReader.h"
#include <windowsx.h>
#include <commctrl.h>

LRESULT CALLBACK GlobalWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    SvgViewer* pViewer = nullptr;

    if (message == WM_CREATE)
    {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pViewer = reinterpret_cast<SvgViewer*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pViewer);
    }
    else
    {
        pViewer = reinterpret_cast<SvgViewer*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    if (pViewer)
    {
        return pViewer->handleMessage(hWnd, message, wParam, lParam);
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

SvgViewer::SvgViewer()
{
    m_hWnd = NULL;
    gdiplusToken = 0;
    screenWidth = 800;
    screenHeight = 600;

    zoomFactor = 1.0f;
    rotationAngle = 0.0f;
    translationOffset = PointF(0.0f, 0.0f);

    isDragging = false;
    lastMousePos = { 0, 0 };
}

SvgViewer::~SvgViewer()
{
}

void SvgViewer::run()
{
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {};
    wc.lpfnWndProc = GlobalWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TEXT("SvgViewerWindow");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    m_hWnd = CreateWindow(
        wc.lpszClassName,
        TEXT("SVG Viewer C++"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, screenWidth, screenHeight,
        NULL,
        NULL,
        hInstance,
        this
    );

    if (m_hWnd == NULL) {
        return;
    }

    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    GdiplusShutdown(gdiplusToken);
}

LRESULT SvgViewer::handleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_LBUTTONDOWN:
    {
        isDragging = true;
        lastMousePos.x = GET_X_LPARAM(lParam);
        lastMousePos.y = GET_Y_LPARAM(lParam);
        SetCapture(hWnd);
        return 0;
    }

    case WM_MOUSEMOVE:
    {
        if (isDragging)
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            float dx = (float)(x - lastMousePos.x);
            float dy = (float)(y - lastMousePos.y);

            translationOffset.X += dx;
            translationOffset.Y += dy;

            lastMousePos.x = x;
            lastMousePos.y = y;

            InvalidateRect(hWnd, NULL, FALSE);
        }
        return 0;
    }

    case WM_LBUTTONUP:
    {
        if (isDragging)
        {
            isDragging = false;
            ReleaseCapture();
        }
        return 0;
    }

    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        int width = clientRect.right - clientRect.left;
        int height = clientRect.bottom - clientRect.top;
        if (width > 0 && height > 0)
        {
            Bitmap offscreenBitmap(width, height);
            Graphics graphics_buffer(&offscreenBitmap);
            render(graphics_buffer);
            Graphics graphics_window(hdc);
            graphics_window.DrawImage(&offscreenBitmap, 0, 0);
        }
        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_SIZE:
    {
        screenWidth = LOWORD(lParam);
        screenHeight = HIWORD(lParam);
        InvalidateRect(hWnd, NULL, FALSE);
        return 0;
    }

    case WM_MOUSEWHEEL:
    {
        float zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

        float scaleRatio = (zDelta > 0) ? 1.1f : 0.9f;

        float oldZoom = zoomFactor;
        float newZoom = oldZoom * scaleRatio;

        if (newZoom < 0.05f) newZoom = 0.05f;
        if (newZoom > 20.0f) newZoom = 20.0f;

        float effectiveRatio = newZoom / oldZoom;

        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);

        ScreenToClient(hWnd, &pt);

        float centerX = (float)screenWidth * 0.5f;
        float centerY = (float)screenHeight * 0.5f;

        float vectorX = (float)pt.x - centerX - translationOffset.X;
        float vectorY = (float)pt.y - centerY - translationOffset.Y;

        translationOffset.X += vectorX * (1.0f - effectiveRatio);
        translationOffset.Y += vectorY * (1.0f - effectiveRatio);
        zoomFactor = newZoom;

        InvalidateRect(hWnd, NULL, FALSE);
        return 0;
    }

    case WM_KEYDOWN:
    {
        if (wParam == VK_LEFT || wParam == VK_UP) {
            rotationAngle -= 5.0f;
        }
        else if (wParam == VK_RIGHT || wParam == VK_DOWN) {
            rotationAngle += 5.0f;
        }
        else if (wParam == 'R' || wParam == 0x52) {
            zoomFactor = 1.0f;
            rotationAngle = 0.0f;
            translationOffset = PointF(0.0f, 0.0f);
        }

        if (rotationAngle > 360.0f) rotationAngle -= 360.0f;
        if (rotationAngle < -360.0f) rotationAngle += 360.0f;

        InvalidateRect(hWnd, NULL, FALSE);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void SvgViewer::loadSvg(const string& filePath)
{
    string svgContent = FileReader::ReadFileToString(filePath);

    Parser parser;
    document = parser.parseSVG(svgContent);

    InvalidateRect(m_hWnd, NULL, TRUE);
}

void SvgViewer::render(Graphics& graphics)
{
    graphics.Clear(Color(255, 255, 255, 255));
    graphics.SetSmoothingMode(SmoothingModeHighQuality);
    graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    graphics.SetPageUnit(UnitPixel);
    graphics.SetPageScale(1.0f);
    PointF center((REAL)screenWidth * 0.5f, (REAL)screenHeight * 0.5f);

    graphics.TranslateTransform(translationOffset.X, translationOffset.Y);
    graphics.TranslateTransform(center.X, center.Y);
    graphics.RotateTransform(rotationAngle);
    graphics.ScaleTransform(zoomFactor, zoomFactor);
    graphics.TranslateTransform(-center.X, -center.Y);

    if (document) {
        document->draw(graphics);
    }
}

void SvgViewer::handleInput()
{
    if (GetAsyncKeyState('A') & 0x8000) {
        translationOffset.X -= 5.0f;
    }
    if (GetAsyncKeyState('D') & 0x8000) {
        translationOffset.X += 5.0f;
    }
    if (GetAsyncKeyState('W') & 0x8000) {
        translationOffset.Y -= 5.0f;
    }
    if (GetAsyncKeyState('S') & 0x8000) {
        translationOffset.Y += 5.0f;
    }
}