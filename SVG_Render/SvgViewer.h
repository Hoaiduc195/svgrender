#pragma once
#include <memory>
#include <string>
#include <gdiplus.h>
//#include "SvgDocument.h"
using namespace std;
using namespace Gdiplus;

class SvgViewer {
private:
	//unique_ptr<SvgDocument> document;
	int screenHeight;
	int screenWidth;

public:
	void run();
	void loadSvg(const string& filePath);
	void handleOutput();
	void render(Graphics* graphics);
};