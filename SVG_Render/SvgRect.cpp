#include "SvgRect.h"


SvgRect::SvgRect(float x, float y, float width, float height, float rx, float ry) : x(x), y(y), width(width), height(height), rx(rx), ry(ry) {
    cout << "SvgRect created." << endl;
}

void SvgRect::draw(Graphics& graphics){
    /*Pen pen(stroke); 
    SolidBrush brush(fill);*/ //ko hieu cai nay nha
    SolidBrush brush(Color(
        static_cast<BYTE>(fillOpacity * 255),
        fill.GetR(), fill.GetG(), fill.GetB()));
    Pen pen(Color(
        static_cast<BYTE>(strokeOpacity * 255),
        stroke.GetR(), stroke.GetG(), stroke.GetB()), strokeWidth);
    graphics.FillRectangle(&brush, x, y, width, height);
    graphics.DrawRectangle(&pen, x, y, width, height);
}
