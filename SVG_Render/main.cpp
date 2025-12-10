#include "SvgViewer.h"

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    SvgViewer viewer;
    viewer.loadSvg("Assets/svg-01.xml"); 
    viewer.run();
    return 0;
}