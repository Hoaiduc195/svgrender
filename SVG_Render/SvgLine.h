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
        void accept(Renderer& renderer) override;

        float getX1() const { return x1; }
        float getY1() const { return y1; }
        float getX2() const { return x2; }
        float getY2() const { return y2; }
};