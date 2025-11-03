#pragma once
#include "framework.h"
#include "SvgDocument.h"
#include "Parser.h"
#include "FileReader.h"

class SvgViewer {
private:
    unique_ptr<SvgDocument> document;
    int screenHeight;
    int screenWidth;

    HWND m_hWnd;            
    ULONG_PTR gdiplusToken; 

    float zoomFactor;
    float rotationAngle;
    PointF translationOffset;

public:
    SvgViewer();
    ~SvgViewer();

    void run();
    LRESULT handleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    void loadSvg(const string& filePath);
    void handleInput();
    void render(Graphics& graphics);
};