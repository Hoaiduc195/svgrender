#include "SvgLine.h"

SvgLine::SvgLine(float x1, float y1, float x2, float y2) : x1(x1), x2(x2), y1(y1), y2(y2) {
    cout << "SvgLine created." << endl;
}

void SvgLine::draw(Graphics& graphics){

	graphics.SetSmoothingMode(SmoothingModeHighQuality);
	graphics.SetCompositingQuality(CompositingQualityHighQuality);
	graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
	graphics.SetPixelOffsetMode(PixelOffsetModeHighQuality);
	graphics.SetPageUnit(UnitPixel);
	graphics.SetPageScale(1.0f);

    Pen pen(stroke, strokeWidth); 
    graphics.DrawLine(&pen, x1, y1, x2, y2);
}