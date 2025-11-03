#include "SvgViewer.h"
#include "SvgDocument.h"
#include "Parser.h"
#include "FileReader.h"


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
    bool running = true;
    while (running)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!running)
            break;

        handleInput();
        InvalidateRect(m_hWnd, NULL, FALSE);
    }

    GdiplusShutdown(gdiplusToken);
}

LRESULT SvgViewer::handleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        Graphics graphics(hdc);

        render(graphics);

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_SIZE:
    {
        screenWidth = LOWORD(lParam);
        screenHeight = HIWORD(lParam);
        return 0;
    }

    case WM_MOUSEWHEEL:
    {
        float zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        if (zDelta > 0)
            zoomFactor *= 1.1f;
        else
            zoomFactor *= 0.9f;

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

    float centerX = screenWidth / 2.0f;
    float centerY = screenHeight / 2.0f;

    graphics.TranslateTransform(centerX, centerY);
    graphics.TranslateTransform(translationOffset.X, translationOffset.Y);
    graphics.ScaleTransform(zoomFactor, zoomFactor);
    graphics.RotateTransform(rotationAngle);

    if (document) {
        document->draw(graphics);
    }
}

void SvgViewer::handleInput()
{
    if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
        translationOffset.X -= 5.0f;
    }
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
        translationOffset.X += 5.0f;
    }
    if (GetAsyncKeyState(VK_UP) & 0x8000) {
        translationOffset.Y -= 5.0f;
    }
    if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
        translationOffset.Y += 5.0f;
    }
}