#include "SvgRect.h"
#include "Renderer.h"

SvgRect::SvgRect(float x, float y, float width, float height, float rx, float ry) : x(x), y(y), width(width), height(height), rx(rx), ry(ry) {
    cout << "SvgRect created." << endl;
}

float SvgRect::getX() const {
    return x; 
}
float SvgRect::getY() const {
    return y; 
}
float SvgRect::getWidth() const {
    return width; 
}
float SvgRect::getHeight() const { 
    return height; 
}
float SvgRect::getRx() const { 
    return rx; 
}
float SvgRect::getRy() const { 
    return ry; 
}
RectF SvgRect::getBoundingBox() const {
    GraphicsPath path;
    path.AddRectangle(RectF(x, y, width, height));

    Matrix matrix;
    SetGdiMatrix(getTransform(), matrix);
    path.Transform(&matrix);

    RectF bounds;
    path.GetBounds(&bounds);
    return bounds;
}

void SvgRect::accept(Renderer& renderer) const {
    renderer.render(*this);
}
