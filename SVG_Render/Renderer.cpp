#include "Renderer.h"
#include "SvgRect.h"
#include "SvgCircle.h"
#include "SvgEllipse.h"
#include "SvgLine.h"
#include "SvgPolygon.h"
#include "SvgPolyline.h"
#include "SvgText.h"

using namespace Gdiplus;

Renderer::Renderer(Graphics& g) : g(g) {}

void Renderer::render(const SvgRect& r) {
    Pen pen(Color(static_cast<BYTE>(r.getStrokeOpacity()*255), r.getStroke().GetR(), r.getStroke().GetG(), r.getStroke().GetB()), r.getStrokeWidth());
    SolidBrush brush(Color(static_cast<BYTE>(r.getFillOpacity()*255), r.getFill().GetR(), r.getFill().GetG(), r.getFill().GetB()));
    g.FillRectangle(&brush, r.getX(), r.getY(), r.getWidth(), r.getHeight());
    g.DrawRectangle(&pen, r.getX(), r.getY(), r.getWidth(), r.getHeight());
}

void Renderer::render(const SvgCircle& c) {
    Pen pen(Color(static_cast<BYTE>(c.getStrokeOpacity()*255), c.getStroke().GetR(), c.getStroke().GetG(), c.getStroke().GetB()), c.getStrokeWidth());
    SolidBrush brush(Color(static_cast<BYTE>(c.getFillOpacity()*255), c.getFill().GetR(), c.getFill().GetG(), c.getFill().GetB()));
    float cx = c.getCx();
    float cy = c.getCy();
    float rrad = c.getR();
    g.FillEllipse(&brush, cx - rrad, cy - rrad, rrad * 2, rrad * 2);
    g.DrawEllipse(&pen, cx - rrad, cy - rrad, rrad * 2, rrad * 2);
}

void Renderer::render(const SvgEllipse& e) {
    Pen pen(Color(static_cast<BYTE>(e.getStrokeOpacity()*255), e.getStroke().GetR(), e.getStroke().GetG(), e.getStroke().GetB()), e.getStrokeWidth());
    SolidBrush brush(Color(static_cast<BYTE>(e.getFillOpacity()*255), e.getFill().GetR(), e.getFill().GetG(), e.getFill().GetB()));
    g.FillEllipse(&brush, e.getCx() - e.getRx(), e.getCy() - e.getRy(), e.getRx() * 2, e.getRy() * 2);
    g.DrawEllipse(&pen, e.getCx() - e.getRx(), e.getCy() - e.getRy(), e.getRx() * 2, e.getRy() * 2);
}

void Renderer::render(const SvgLine& l) {
    Pen pen(Color(static_cast<BYTE>(l.getStrokeOpacity()*255), l.getStroke().GetR(), l.getStroke().GetG(), l.getStroke().GetB()), l.getStrokeWidth());
    g.DrawLine(&pen, l.getX1(), l.getY1(), l.getX2(), l.getY2());
}

void Renderer::render(const SvgPolygon& p) {
    const auto& pts = p.getPoints();
    if (pts.size() < 2) return;
    std::vector<PointF> gdiPoints;
    for (const auto& v : pts) gdiPoints.emplace_back(v.x, v.y);

    if (p.getFillOpacity() > 0) {
        SolidBrush brush(Color(static_cast<BYTE>(p.getFillOpacity()*255), p.getFill().GetR(), p.getFill().GetG(), p.getFill().GetB()));
        g.FillPolygon(&brush, gdiPoints.data(), (INT)gdiPoints.size(), FillModeAlternate);
    }

    if (p.getStrokeOpacity() > 0 && p.getStrokeWidth() > 0) {
        Pen pen(Color(static_cast<BYTE>(p.getStrokeOpacity()*255), p.getStroke().GetR(), p.getStroke().GetG(), p.getStroke().GetB()), p.getStrokeWidth());
        g.DrawPolygon(&pen, gdiPoints.data(), (INT)gdiPoints.size());
    }
}

void Renderer::render(const SvgPolyline& p) {
    const auto& pts = p.getPoints();
    if (pts.size() < 2) return;
    std::vector<PointF> gdiPoints;
    for (const auto& v : pts) gdiPoints.emplace_back(v.x, v.y);

    Pen pen(Color(static_cast<BYTE>(p.getStrokeOpacity()*255), p.getStroke().GetR(), p.getStroke().GetG(), p.getStroke().GetB()), p.getStrokeWidth());
    pen.SetLineJoin(LineJoinMiter);
    pen.SetStartCap(LineCapFlat);
    pen.SetEndCap(LineCapFlat);

    if (p.getFillOpacity() > 0.0f) {
        SolidBrush brush(Color(static_cast<BYTE>(p.getFillOpacity()*255), p.getFill().GetR(), p.getFill().GetG(), p.getFill().GetB()));
        g.FillPolygon(&brush, gdiPoints.data(), (INT)gdiPoints.size(), FillModeAlternate);
    }

    g.DrawLines(&pen, gdiPoints.data(), (INT)gdiPoints.size());
}

void Renderer::render(const SvgText& t) {
    StringFormat format;
    FontFamily fontFamily(L"Times New Roman");
    Font font(&fontFamily, t.getFontSize(), FontStyleRegular, UnitPixel);
    SolidBrush brush(Color(static_cast<BYTE>(t.getFillOpacity() * 255), t.getFill().GetR(), t.getFill().GetG(), t.getFill().GetB()));

    RectF layoutRect(t.getX(), t.getY() - font.GetHeight(&g), 1000, font.GetHeight(&g));
    g.DrawString(std::wstring(t.getContent().begin(), t.getContent().end()).c_str(), -1, &font, layoutRect, &format, &brush);
}
