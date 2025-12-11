#include "SvgViewer.h"
#include <commdlg.h>
#include <string>


string RemoveQuotes(string str) {
    if (str.length() >= 2 && str.front() == '"' && str.back() == '"') {
        return str.substr(1, str.length() - 2);
    }
    return str;
}

string OpenFileDialog(HWND owner = NULL) {
    OPENFILENAMEA ofn;
    char szFile[260];
    ZeroMemory(szFile, sizeof(szFile));

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "SVG Files\0*.svg;*.xml\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn) == TRUE) {
        return string(ofn.lpstrFile);
    }
    return "";
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    SvgViewer viewer;
    string filePath = lpCmdLine;

    filePath = RemoveQuotes(filePath);

    if (filePath.empty()) {
        filePath = OpenFileDialog(NULL);

        if (filePath.empty()) {
            return 0;
        }
    }

    if (!filePath.empty()) {
        viewer.loadSvg(filePath);
    }

    viewer.run();

    return 0;
}