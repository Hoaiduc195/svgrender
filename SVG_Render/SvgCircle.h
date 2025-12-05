#pragma once
#include "SvgElement.h"
#include "framework.h"

class Renderer;

class SvgCircle : public SvgElement {
    private:
        float cx;
        float cy;
        float r;
    public:
        SvgCircle(float centerX, float centerY, float radius);
        void accept(Renderer& renderer) override;

        float getCx() const { return cx; }
        float getCy() const { return cy; }
        float getR() const { return r; }
};