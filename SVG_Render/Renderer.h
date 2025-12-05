#pragma once
#include "framework.h"
#include "SvgRect.h"
#include "SvgCircle.h"
#include "SvgEllipse.h"
#include "SvgLine.h"
#include "SvgPolygon.h"
#include "SvgPolyline.h"
#include "SvgText.h"


class Renderer {
public:
    Renderer(Graphics& g);

    void render(const SvgRect& r);
    void render(const SvgCircle& c);
    void render(const SvgEllipse& e);
    void render(const SvgLine& l);
    void render(const SvgPolygon& p);
    void render(const SvgPolyline& p);
    void render(const SvgText& t);

private:
    Graphics& g;
};
