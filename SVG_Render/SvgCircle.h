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
        SvgCircle(float centerX, float centerY, float radius) : cx(centerX), cy(centerY), r(radius) {
            cout << "SvgCircle created." << endl;
        }

        void draw(Gdiplus::Graphics& graphics) override {
            Pen pen(Gdiplus::Color(255, 0, 0, 0)); 
            SolidBrush brush(Gdiplus::Color(255, 255, 0, 0)); 
            graphics.FillEllipse(&brush, cx - r, cy - r, r * 2, r * 2);
            graphics.DrawEllipse(&pen, cx - r, cy - r, r * 2, r * 2);
        }
};