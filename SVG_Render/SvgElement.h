#pragma once
#include<iostream>
#include "framework.h"

using namespace Gdiplus;

class SvgElement {
    protected:
        //string id; ??
        Color fill;
        Color stroke;
        float strokeWidth;
        float strokeOpacity;
        float fillOpacity;

    public:
        virtual void draw(Graphics& graphics) = 0;
        virtual ~SvgElement() = default;
        SvgElement();
        // Setters
        void setFill(const Color& fillColor);
		void setStroke(const Color& strokeColor);
		void setStrokeWidth(float width);
		void setStrokeOpacity(float opacity);
		void setFillOpacity(float opacity);
};