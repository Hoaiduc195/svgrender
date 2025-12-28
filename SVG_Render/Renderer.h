#pragma once
#include "framework.h"
#include "SvgRect.h"
#include "SvgCircle.h"
#include "SvgEllipse.h"
#include "SvgLine.h"
#include "SvgPolygon.h"
#include "SvgPolyline.h"
#include "SvgText.h"
#include "SvgPath.h"
#include "SvgGroup.h"
#include "SvgDocument.h"


class Renderer {
public:
    Renderer(Graphics& g) : g(g) {}
    Renderer(Graphics& graphics, const SvgDocument* document)
        : g(graphics), doc(document) {
    }
    void render(const SvgRect& r);
    void render(const SvgCircle& c);
    void render(const SvgEllipse& e);
    void render(const SvgLine& l);
    void render(const SvgPolygon& p);
    void render(const SvgPolyline& p);
    void render(const SvgText& t);
    void render(const SvgPath& p);
    void render(const SvgGroup& g);
    
private:
    Graphics& g;
    const SvgDocument* doc;
};
