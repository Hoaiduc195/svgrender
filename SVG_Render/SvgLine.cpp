#include "SvgLine.h"

SvgLine::SvgLine(float x1, float x2, float y1, float y2) : x1(x1), x2(x2), y1(y1), y2(y2) {
    cout << "SvgLine created." << endl;
}

void SvgLine::draw(Graphics& graphics){
    Pen pen(stroke); 
    graphics.DrawLine(&pen, x1, y1, x2, y2);
}