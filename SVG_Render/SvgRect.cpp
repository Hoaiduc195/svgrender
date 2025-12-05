#include "SvgRect.h"
#include "Renderer.h"

SvgRect::SvgRect(float x, float y, float width, float height, float rx, float ry) : x(x), y(y), width(width), height(height), rx(rx), ry(ry) {
    cout << "SvgRect created." << endl;
}

void SvgRect::accept(Renderer& renderer) {
    renderer.render(*this);
}
