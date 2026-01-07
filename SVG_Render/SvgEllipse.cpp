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
RectF SvgEllipse::getBoundingBox() const {
    GraphicsPath path;
    path.AddEllipse(cx - rx, cy - ry, rx * 2, ry * 2);

    Matrix matrix;
    SetGdiMatrix(getTransform(), matrix);
    path.Transform(&matrix);

    RectF bounds;
    path.GetBounds(&bounds);
    return bounds;
}

void SvgEllipse::accept(Renderer& renderer) const {
    renderer.render(*this);
}

