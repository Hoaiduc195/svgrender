#pragma
#include<iostream>
#include "SvgElement.h"
#include "framework.h"

using namespace Gdiplus;

class SvgEllipse : public SvgElement {
    private:
        float cx;
        float cy;
        float rx;
        float ry;
    public:
        SvgEllipse(float cx, float cy, float rx, float ry);
        void draw(Graphics& graphics) override;
};