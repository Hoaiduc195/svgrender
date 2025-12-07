#include "SvgEllipse.h"
#include "Renderer.h"

SvgEllipse::SvgEllipse(float cx, float cy, float rx, float ry) : cx(cx), cy(cy), rx(rx), ry(ry) {
    cout << "SvgEllipse created." << endl;
}

float SvgEllipse::getCx() const {
    return cx; 
}
float SvgEllipse::getCy() const {
    return cy; 
}
float SvgEllipse::getRx() const {
    return rx; 
}
float SvgEllipse::getRy() const {
    return ry; 
}


void SvgEllipse::accept(Renderer& renderer) {
    renderer.render(*this);
}

