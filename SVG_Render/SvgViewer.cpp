#include "SvgViewer.h"
//#include "SvgDocument.h" 
#include "Parser.h"    
//#include "FileReader.h" 
#include <windows.h>     

// this line is for test

SvgViewer::SvgViewer() {
    screenHeight = 480;
    screenWidth = 640;
}

SvgViewer::~SvgViewer() {
    // pass
}

void SvgViewer::loadSvg(const string& filePath) {
    string svgContent = FileReader::ReadFileToString(filePath);

    // 2. Parse
    Parser parser;
    document = parser.parseXMLDocument(svgContent);
}

void SvgViewer::render(Graphics* graphics) {
    if (document) { // Kiểm tra xem document có tồn tại không

        // 1. Thiết lập "Camera" (pan, zoom, rotate)
        //    graphics->TranslateTransform(...);
        //    graphics->ScaleTransform(...);
        //    graphics->RotateTransform(...);

        // 2. Ra lệnh vẽ
        document->draw(graphics); // <-- Bây giờ compiler biết document->draw() là gì
    }
}

void SvgViewer::handleInput() {
    // Xử lý input (chuột, bàn phím) để cập nhật pan, zoom, rotate...
    // ...
}

void SvgViewer::run() {
    // Chứa vòng lặp WinMain...
    // ...
}