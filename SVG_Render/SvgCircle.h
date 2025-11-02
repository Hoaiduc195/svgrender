#pragma once
#include "SvgElement.h"
#include "framework.h"
using namespace Gdiplus;

class SvgCircle : public SvgElement {
    private:
        float cx;
        float cy;
        float r;
    public:
        SvgCircle(float centerX, float centerY, float radius);
        void draw(Graphics& graphics) override;
};