#include "SvgCircle.h"


SvgCircle::SvgCircle(float centerX, float centerY, float radius) : cx(centerX), cy(centerY), r(radius) {
    cout << "SvgCircle created." << endl;
}

void SvgCircle::draw(Graphics& graphics){
    Pen pen(stroke); 
    SolidBrush brush(fill); 
    graphics.FillEllipse(&brush, cx - r, cy - r, r * 2, r * 2);
    graphics.DrawEllipse(&pen, cx - r, cy - r, r * 2, r * 2);
}
