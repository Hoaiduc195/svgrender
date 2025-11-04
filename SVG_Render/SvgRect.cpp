#include "SvgRect.h"


SvgRect::SvgRect(float x, float y, float width, float height, float rx, float ry) : x(x), y(y), width(width), height(height), rx(rx), ry(ry) {
    cout << "SvgRect created." << endl;
}

void SvgRect::draw(Graphics& graphics){
    Pen pen(stroke); 
    SolidBrush brush(fill);
    graphics.FillRectangle(&brush, x, y, width, height);
    graphics.DrawRectangle(&pen, x, y, width, height);
}
