#pragma once
#include<iostream>
#include <gdiplus.h>
#include "SvgElement.h"
#include "tinyxml2.h"
#include "framework.h"

using namespace Gdiplus;

class SvgRect : public SvgElement {
    private:
        float x;
        float y;
        float width;
        float height;
        float rx;
        float ry;
        Color fillColor;
    public:
        SvgRect(float x, float y, float width, float height, float rx, float ry, const Color& fill);
        void draw(Graphics& graphics) override;
};