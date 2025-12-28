#include "SvgLine.h"
#include "Renderer.h"

SvgLine::SvgLine(float x1, float y1, float x2, float y2) : x1(x1), x2(x2), y1(y1), y2(y2) {
    cout << "SvgLine created." << endl;
}

float SvgLine::getX1() const {
    return x1; 
}
float SvgLine::getY1() const {
    return y1; 
}
float SvgLine::getX2() const {
    return x2; 
}
float SvgLine::getY2() const {
    return y2; 
}
RectF SvgLine::getBoundingBox() const {
    float minX = min(x1, x2);
    float minY = min(y1, y2); 
    return RectF(minX, minY, abs(x1 - x2), abs(y1 - y2));
}
void SvgLine::accept(Renderer& renderer) const {
    renderer.render(*this);
}