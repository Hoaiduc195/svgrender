#include<iostream>
#include <gdiplus.h>
#include "SvgElement.h"
#include "SvgEllipse.h"
#include "tinyxml2.h"

using namespace std;
using namespace Gdiplus;


    SvgEllipse::SvgEllipse(float cx, float cy, float rx, float ry) : cx(cx), cy(cy), rx(rx), ry(ry) {
        cout << "SvgEllipse created." << endl;
    }

    void SvgEllipse::draw(Gdiplus::Graphics& graphics)  {
        Pen pen(Gdiplus::Color(255, 0, 0, 0));
        SolidBrush brush(Gdiplus::Color(255, 255, 0, 0));
            graphics.FillEllipse(&brush, cx - rx, cy - ry, rx * 2, ry * 2);
        graphics.DrawEllipse(&pen, cx - rx, cy - ry, rx * 2, ry * 2);
    }