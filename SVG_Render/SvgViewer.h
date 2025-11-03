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

public:
	void run();
	void loadSvg(const string& filePath);
	void handleOutput();
	void render(Graphics* graphics);
};