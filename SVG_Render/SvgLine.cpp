#include "SvgLine.h"
#include "Renderer.h"

SvgLine::SvgLine(float x1, float y1, float x2, float y2) : x1(x1), x2(x2), y1(y1), y2(y2) {
    cout << "SvgLine created." << endl;
}

void SvgLine::accept(Renderer& renderer) {
    renderer.render(*this);
}