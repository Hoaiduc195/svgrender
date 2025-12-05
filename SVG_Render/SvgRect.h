#pragma once
#include "SvgElement.h"
#include "framework.h"

class Renderer; 

class SvgRect : public SvgElement {
    private:
        float x;
        float y;
        float width;
        float height;
        float rx;
        float ry;
    public:
        SvgRect(float x, float y, float width, float height, float rx, float ry);
        void accept(Renderer& renderer) override;

        // Getters for renderer
        float getX() const { return x; }
        float getY() const { return y; }
        float getWidth() const { return width; }
        float getHeight() const { return height; }
        float getRx() const { return rx; }
        float getRy() const { return ry; }
};