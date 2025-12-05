#include "SvgEllipse.h"
#include "Renderer.h"

SvgEllipse::SvgEllipse(float cx, float cy, float rx, float ry) : cx(cx), cy(cy), rx(rx), ry(ry) {
    cout << "SvgEllipse created." << endl;
}

void SvgEllipse::accept(Renderer& renderer) {
    renderer.render(*this);
}