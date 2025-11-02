#include <windows.h>
#include "framework.h"
using namespace Gdiplus;



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    /*.........................*/

    GdiplusShutdown(gdiplusToken);
    return 0;
}
