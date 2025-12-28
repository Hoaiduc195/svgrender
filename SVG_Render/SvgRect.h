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
        void accept(Renderer& renderer) const override;

        // Getters
        float getX() const;
        float getY() const;
        float getWidth() const;
        float getHeight() const;
        float getRx() const;
        float getRy() const;
        RectF getBoundingBox() const override;
};