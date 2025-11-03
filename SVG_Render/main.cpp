#include <windows.h>
#include "SvgViewer.h"

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    SvgViewer viewer;
    viewer.loadSvg("SVG_Render/Assets/sample.xml");
    viewer.run();
    return 0;
}