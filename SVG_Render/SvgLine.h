#pragma once
#include "SvgElement.h"
#include "framework.h"

class Renderer;

class SvgLine : public SvgElement {
    private:
        float x1;
        float y1;
        float x2;
        float y2;
    public:
        SvgLine(float x1, float y1, float x2, float y2);
        void accept(Renderer& renderer) const override;
            
        //Getters
        float getX1() const;
        float getY1() const;
        float getX2() const;
        float getY2() const;
        RectF getBoundingBox() const override;
};