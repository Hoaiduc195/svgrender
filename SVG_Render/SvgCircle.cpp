#include "SvgCircle.h"


SvgCircle::SvgCircle(float centerX, float centerY, float radius) : cx(centerX), cy(centerY), r(radius) {
    cout << "SvgCircle created." << endl;
}

void SvgCircle::draw(Graphics& graphics){

    graphics.SetSmoothingMode(SmoothingModeHighQuality);
    graphics.SetCompositingQuality(CompositingQualityHighQuality);
    graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    graphics.SetPixelOffsetMode(PixelOffsetModeHighQuality);
    graphics.SetPageUnit(UnitPixel);
    graphics.SetPageScale(1.0f);
    
    Pen pen(Color(static_cast<BYTE>(strokeOpacity*255), stroke.GetR(), stroke.GetG(), stroke.GetB()), strokeWidth); 
    SolidBrush brush(Color(static_cast<BYTE>(fillOpacity*255), fill.GetR(), fill.GetG(), fill.GetB())); 
    
    graphics.FillEllipse(&brush, cx - r, cy - r, r * 2, r * 2);
    graphics.DrawEllipse(&pen, cx - r, cy - r, r * 2, r * 2);
}
