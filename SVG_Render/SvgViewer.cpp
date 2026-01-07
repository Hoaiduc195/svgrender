#include "SvgViewer.h"
#include "SvgDocument.h"
#include "Parser.h"
#include "FileReader.h"
#include <windowsx.h>
#include <commctrl.h>
#include <shobjidl.h>

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
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    GdiplusShutdown(gdiplusToken);
}

#include <shobjidl.h> // Required for IFileSaveDialog

wstring SvgViewer::showSaveDialog() {
    IFileSaveDialog* pSaveDialog = nullptr;
    std::wstring filePath = L"";

    // Create the FileSaveDialog object
    HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
        IID_IFileSaveDialog, reinterpret_cast<void**>(&pSaveDialog));

    if (SUCCEEDED(hr)) {
        // Set the file types/filters to PNG
        COMDLG_FILTERSPEC pngFilter[] = { { L"PNG Image", L"*.png" } };
        pSaveDialog->SetFileTypes(1, pngFilter);
        pSaveDialog->SetDefaultExtension(L"png");
        pSaveDialog->SetFileName(L"drawing.png"); // Suggest a default name

        // Show the dialog
        hr = pSaveDialog->Show(m_hWnd);

        if (SUCCEEDED(hr)) {
            IShellItem* pItem;
            hr = pSaveDialog->GetResult(&pItem);
            if (SUCCEEDED(hr)) {
                PWSTR pszFilePath;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                if (SUCCEEDED(hr)) {
                    filePath = pszFilePath;
                    CoTaskMemFree(pszFilePath); // Free the memory allocated by the shell
                }
                pItem->Release();
            }
        }
        pSaveDialog->Release();
    }
    return filePath;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0;          // number of image encoders
    UINT size = 0;         // size of the image encoder array in bytes

    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;

    Gdiplus::ImageCodecInfo* pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL) return -1;

    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }
    }

    free(pImageCodecInfo);
    return -1; // Failure
}

void SvgViewer::exportToPng(const wstring& filePath) {
    Gdiplus::Bitmap bitmap(screenWidth, screenHeight, PixelFormat32bppARGB);
    Gdiplus::Graphics g(&bitmap);
    render(g);

    CLSID pngClsid;
    if (GetEncoderClsid(L"image/png", &pngClsid) != -1) {
        Gdiplus::Status status = bitmap.Save(filePath.c_str(), &pngClsid, NULL);

        if (status == Gdiplus::Ok) {
            MessageBoxW(m_hWnd, (L"Saved to: " + filePath).c_str(), L"Success", MB_OK);
        }
        else {
            MessageBoxW(m_hWnd, L"GDI+ Save Failed! Check folder permissions.", L"Error", MB_ICONERROR);
        }
    }
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
        }else if (wParam == 'E') {
            std::wstring path = showSaveDialog();
            if (!path.empty()) {
                exportToPng(path);
            }
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

    float objectCenterX = 0.0f;
    float objectCenterY = 0.0f;

    if (document) {
        RectF bounds = document->getBoundingBox();
        objectCenterX = bounds.X + bounds.Width * 0.5f;
        objectCenterY = bounds.Y + bounds.Height * 0.5f;
    }

    float screenCenterX = (float)screenWidth * 0.5f;
    float screenCenterY = (float)screenHeight * 0.5f;

    graphics.TranslateTransform(screenCenterX + translationOffset.X, screenCenterY + translationOffset.Y);
    graphics.RotateTransform(rotationAngle);
    graphics.ScaleTransform(zoomFactor, zoomFactor);
    graphics.TranslateTransform(-objectCenterX, -objectCenterY);

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