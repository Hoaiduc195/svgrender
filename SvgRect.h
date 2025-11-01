#include<iostream>
#include <gdiplus.h>
#include "SvgElement.h"
#include "tinyxml2.h"

using namespace std;
using namespace Gdiplus;

class SvgRect : public SvgElement {
    private:
        float x;
        float y;
        float width;
        float height;
        float rx;
        float ry;
    public:
        SvgRect(float x, float y, float width, float height, float rx, float ry) : x(x), y(y), width(width), height(height), rx(rx), ry(ry) {
            cout << "SvgRect created." << endl;
        }

        void draw(Gdiplus::Graphics& graphics) override {
            Pen pen(Gdiplus::Color(255, 0, 0, 0)); 
            SolidBrush brush(Gdiplus::Color(255, 255, 0, 0));
            graphics.FillRectangle(&brush, x, y, width, height);
            graphics.DrawRectangle(&pen, x, y, width, height);
        }
};