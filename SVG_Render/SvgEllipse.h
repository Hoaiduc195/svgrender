#include<iostream>
#include <gdiplus.h>
#include "SvgElement.h"
#include "tinyxml2.h"

using namespace std;
using namespace Gdiplus;

class SvgEllipse : public SvgElement {
    private:
        float cx;
        float cy;
        float rx;
        float ry;
    public:
        SvgEllipse(float cx, float cy, float rx, float ry);
        void draw(Gdiplus::Graphics& graphics) override;
};