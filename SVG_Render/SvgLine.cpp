#include<iostream>
#include <gdiplus.h>
#include "SvgElement.h"
#include "SvgLine.h"
#include "tinyxml2.h"

using namespace std;
using namespace Gdiplus;

SvgLine::SvgLine(float x1, float x2, float y1, float y2) : x1(x1), x2(x2), y1(y1), y2(y2) {
    cout << "SvgLine created." << endl;
}

void SvgLine::draw(Gdiplus::Graphics& graphics){
    Pen pen(Gdiplus::Color(255, 0, 0, 0)); 
    graphics.DrawLine(&pen, x1, y1, x2, y2);
}