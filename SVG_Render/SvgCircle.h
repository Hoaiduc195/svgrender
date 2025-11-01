#include<iostream>
#include <gdiplus.h>
#include "SvgElement.h"
using namespace std;
using namespace Gdiplus;

class SvgCircle : public SvgElement {
    private:
        float cx;
        float cy;
        float r;
    public:
        SvgCircle(float centerX, float centerY, float radius);
        void draw(Gdiplus::Graphics& graphics) override;
};