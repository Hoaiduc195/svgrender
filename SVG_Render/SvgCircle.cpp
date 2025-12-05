#include "SvgCircle.h"
#include "Renderer.h"

SvgCircle::SvgCircle(float centerX, float centerY, float radius) : cx(centerX), cy(centerY), r(radius) {
    cout << "SvgCircle created." << endl;
}

void SvgCircle::accept(Renderer& renderer) {
    renderer.render(*this);
}
