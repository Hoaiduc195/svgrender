#include<iostream>
#include <gdiplus.h>
#include "SvgElement.h"
#include "SvgCircle.h"
using namespace std;
using namespace Gdiplus;


    SvgCircle::SvgCircle(float centerX, float centerY, float radius) : cx(centerX), cy(centerY), r(radius) {
        cout << "SvgCircle created." << endl;
    }

    void SvgCircle::draw(Gdiplus::Graphics& graphics){
        Pen pen(Gdiplus::Color(255, 0, 0, 0)); 
        SolidBrush brush(Gdiplus::Color(255, 255, 0, 0)); 
        graphics.FillEllipse(&brush, cx - r, cy - r, r * 2, r * 2);
        graphics.DrawEllipse(&pen, cx - r, cy - r, r * 2, r * 2);
    }
