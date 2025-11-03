#include "SvgRect.h"


SvgRect::SvgRect(float x, float y, float width, float height, float rx, float ry, const Color& fill) : x(x), y(y), width(width), height(height), rx(rx), ry(ry), fillColor(fill) {
    cout << "SvgRect created." << endl;
}

void SvgRect::draw(Graphics& graphics){
    Pen pen(Color(255, 0, 0, 0)); 
    SolidBrush brush(Color(255, 255, 0, 0));
    graphics.FillRectangle(&brush, x, y, width, height);
    graphics.DrawRectangle(&pen, x, y, width, height);
}
