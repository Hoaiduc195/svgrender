#pragma once
#include<iostream>
#include <gdiplus.h>
#include "SvgElement.h"
#include "framework.h"

using namespace Gdiplus;

class SvgLine : public SvgElement {
    private:
        float x1;
        float y1;
        float x2;
        float y2;
    public:
        SvgLine(float x1, float x2, float y1, float y2);
        void draw(Graphics& graphics) override;
};