#pragma once
#include "tinyxml2.h"
#include "SvgDocument.h"
#include "SvgElement.h"
#include "SvgCircle.h"
#include "SvgRect.h"
#include "gdipluscolor.h"
//#include <memory>


using namespace std;
using namespace Gdiplus;

class Parser {
public:
    static unique_ptr<SvgDocument> parseSVG(const string& xmlText);
};
