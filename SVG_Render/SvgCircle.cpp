#include "SvgCircle.h"
#include "Renderer.h"

SvgCircle::SvgCircle(float centerX, float centerY, float radius) : cx(centerX), cy(centerY), r(radius) {
    cout << "SvgCircle created." << endl;
}

float SvgCircle::getCx() const {
    return cx; 
}
float SvgCircle::getCy() const {
    return cy; 
}
float SvgCircle::getR() const {
    return r; 
}
RectF SvgCircle::getBoundingBox() const {
    return RectF(cx - r, cy - r, 2 * r, 2 * r);
}
void SvgCircle::accept(Renderer& renderer) const {
    renderer.render(*this);
}
