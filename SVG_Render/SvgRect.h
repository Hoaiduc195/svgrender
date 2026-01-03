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

        // Getters
        float getX() const;
        float getY() const;
        float getWidth() const;
        float getHeight() const;
        float getRx() const;
        float getRy() const;

        void accept(Renderer& renderer) const override;
        RectF getBoundingBox() const override;
};