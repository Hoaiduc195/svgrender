#include "Renderer.h"
#include "SvgRect.h"
#include "SvgCircle.h"
#include "SvgEllipse.h"
#include "SvgLine.h"
#include "SvgPolygon.h"
#include "SvgPolyline.h"
#include "SvgText.h"
#include "Transform.h"


// Helper function to apply transform to graphics context
static void applyTransform(Graphics& graphics, const Transform& transform) {
    Matrix3x3 m = transform.getMatrix();
    Matrix gdipMatrix(
        m.matrix[0][0], m.matrix[0][1],
        m.matrix[1][0], m.matrix[1][1],
        m.matrix[0][2], m.matrix[1][2]
    );
    graphics.MultiplyTransform(&gdipMatrix);
}

Renderer::Renderer(Graphics& g) : g(g) {}

void Renderer::render(const SvgRect& r) {
    GraphicsState state = g.Save();
    
    applyTransform(g, r.getTransform());
    
    Pen pen(Color(static_cast<BYTE>(r.getStrokeOpacity()*255), r.getStroke().GetR(), r.getStroke().GetG(), r.getStroke().GetB()), r.getStrokeWidth());
    SolidBrush brush(Color(static_cast<BYTE>(r.getFillOpacity()*255), r.getFill().GetR(), r.getFill().GetG(), r.getFill().GetB()));
    g.FillRectangle(&brush, r.getX(), r.getY(), r.getWidth(), r.getHeight());
    g.DrawRectangle(&pen, r.getX(), r.getY(), r.getWidth(), r.getHeight());
    
    g.Restore(state);
}

void Renderer::render(const SvgCircle& c) {
    GraphicsState state = g.Save();
    applyTransform(g, c.getTransform());
    
    Pen pen(Color(static_cast<BYTE>(c.getStrokeOpacity()*255), c.getStroke().GetR(), c.getStroke().GetG(), c.getStroke().GetB()), c.getStrokeWidth());
    SolidBrush brush(Color(static_cast<BYTE>(c.getFillOpacity()*255), c.getFill().GetR(), c.getFill().GetG(), c.getFill().GetB()));
    float cx = c.getCx();
    float cy = c.getCy();
    float rrad = c.getR();
    g.FillEllipse(&brush, cx - rrad, cy - rrad, rrad * 2, rrad * 2);
    g.DrawEllipse(&pen, cx - rrad, cy - rrad, rrad * 2, rrad * 2);
    
    g.Restore(state);
}

void Renderer::render(const SvgEllipse& e) {
    GraphicsState state = g.Save();
    applyTransform(g, e.getTransform());
    
    Pen pen(Color(static_cast<BYTE>(e.getStrokeOpacity()*255), e.getStroke().GetR(), e.getStroke().GetG(), e.getStroke().GetB()), e.getStrokeWidth());
    SolidBrush brush(Color(static_cast<BYTE>(e.getFillOpacity()*255), e.getFill().GetR(), e.getFill().GetG(), e.getFill().GetB()));
    g.FillEllipse(&brush, e.getCx() - e.getRx(), e.getCy() - e.getRy(), e.getRx() * 2, e.getRy() * 2);
    g.DrawEllipse(&pen, e.getCx() - e.getRx(), e.getCy() - e.getRy(), e.getRx() * 2, e.getRy() * 2);
    
    g.Restore(state);
}

void Renderer::render(const SvgLine& l) {
    GraphicsState state = g.Save();
    applyTransform(g, l.getTransform());
    
    Pen pen(Color(static_cast<BYTE>(l.getStrokeOpacity()*255), l.getStroke().GetR(), l.getStroke().GetG(), l.getStroke().GetB()), l.getStrokeWidth());
    g.DrawLine(&pen, l.getX1(), l.getY1(), l.getX2(), l.getY2());
    
    g.Restore(state);
}

void Renderer::render(const SvgPolygon& p) {
    GraphicsState state = g.Save();
    applyTransform(g, p.getTransform());
    
    const auto& pts = p.getPoints();
    if (pts.size() < 2) {
        g.Restore(state);
        return;
    }
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
    
    g.Restore(state);
}

void Renderer::render(const SvgPolyline& p) {
    GraphicsState state = g.Save();
    applyTransform(g, p.getTransform());
    
    const auto& pts = p.getPoints();
    if (pts.size() < 2) {
        g.Restore(state);
        return;
    }
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
    
    g.Restore(state);
}

void Renderer::render(const SvgText& t) {
    GraphicsState state = g.Save();
    applyTransform(g, t.getTransform());

    FontFamily fontFamily(L"Times New Roman");
    int fontStyle = FontStyleRegular;
    float fontSize = t.getFontSize();

    std::wstring wContent(t.getContent().begin(), t.getContent().end());

    GraphicsPath path;

    Font tempFont(&fontFamily, fontSize, fontStyle, UnitPixel);
    float yPos = t.getY() - tempFont.GetHeight(&g);
    PointF origin(t.getX(), yPos);

    StringFormat format;
    path.AddString(
        wContent.c_str(),
        -1,
        &fontFamily,
        fontStyle,
        fontSize,
        origin,
        &format
    );

    if (t.getFillOpacity() > 0.0f) {
        Color fillColor(
            static_cast<BYTE>(t.getFillOpacity() * 255),
            t.getFill().GetR(),
            t.getFill().GetG(),
            t.getFill().GetB()
        );
        SolidBrush brush(fillColor);
        g.FillPath(&brush, &path);
    }

    if (t.getStrokeOpacity() > 0.0f && t.getStrokeWidth() > 0.0f) {
        Color strokeColor(
            static_cast<BYTE>(t.getStrokeOpacity() * 255),
            t.getStroke().GetR(),
            t.getStroke().GetG(),
            t.getStroke().GetB()
        );
        Pen pen(strokeColor, t.getStrokeWidth());

        pen.SetLineJoin(LineJoinRound);
        g.DrawPath(&pen, &path);
    }

    g.Restore(state);
}

void Renderer::render(const SvgPath& p) {
    GraphicsState state = g.Save();
    applyTransform(g, p.getTransform());

    const string& d = p.getPathData();
    if (d.empty()) {
        g.Restore(state);
        return;
    }

    GraphicsPath path;
    PointF cur(0, 0);
    PointF start(0, 0);
    bool figureStarted = false;
    size_t i = 0;
    char cmd = 0;

    auto skip = [&]() {
        while (i < d.size() && (isspace(d[i]) || d[i] == ',')) i++;
        };

    auto parseNum = [&]() -> float {
        skip();
        bool neg = false;
        if (i < d.size() && d[i] == '-') {
            neg = true; i++;
        }
        float v = 0;
        while (i < d.size() && isdigit(d[i])) {
            v = v * 10 + (d[i] - '0');
            i++;
        }
        if (i < d.size() && d[i] == '.') {
            i++;
            float mul = 0.1f;
            while (i < d.size() && isdigit(d[i])) {
                v += (d[i] - '0') * mul;
                mul *= 0.1f;
                i++;
            }
        }
        return neg ? -v : v;
        };

    while (i < d.size()) {
        skip();
        if (i >= d.size()) break;

        if (isalpha(d[i])) {
            cmd = d[i++];
            skip();
        }
        else if (cmd == 0) {
            i++; continue;
        }

        bool isRelative = islower(cmd);
        char upperCmd = toupper(cmd);

        switch (upperCmd)
        {
        case 'M': {
            float x = parseNum();
            float y = parseNum();

            if (isRelative) {
                x += cur.X;
                y += cur.Y;
            }

            path.StartFigure();
            cur = PointF(x, y);
            start = cur;
            figureStarted = true;

            cmd = isRelative ? 'l' : 'L';
            break;
        }

        case 'L': {
            float x = parseNum();
            float y = parseNum();

            if (isRelative) {
                x += cur.X;
                y += cur.Y;
            }

            if (figureStarted) path.AddLine(cur, PointF(x, y));
            cur = PointF(x, y);
            break;
        }

        case 'H': {
            float x = parseNum();
            if (isRelative) x += cur.X;

            if (figureStarted) path.AddLine(cur, PointF(x, cur.Y));
            cur.X = x;
            break;
        }

        case 'V': {
            float y = parseNum();
            if (isRelative) y += cur.Y;

            if (figureStarted) path.AddLine(cur, PointF(cur.X, y));
            cur.Y = y;
            break;
        }

        case 'C': {
            float x1 = parseNum(), y1 = parseNum();
            float x2 = parseNum(), y2 = parseNum();
            float x = parseNum(), y = parseNum();

            if (isRelative) {
                x1 += cur.X; y1 += cur.Y;
                x2 += cur.X; y2 += cur.Y;
                x += cur.X; y += cur.Y;
            }

            if (figureStarted)
                path.AddBezier(cur, PointF(x1, y1), PointF(x2, y2), PointF(x, y));

            cur = PointF(x, y);
            break;
        }

        case 'Z': {
            if (figureStarted) {
                path.CloseFigure();
            }
            cur = start;
            break;
        }

        default:
            i++;
            break;
        }
    }

    if (p.getFillOpacity() > 0.f) {
        Color color(
            (BYTE)(p.getFillOpacity() * 255),
            p.getFill().GetR(),
            p.getFill().GetG(),
            p.getFill().GetB()
        );
        SolidBrush brush(color);
        g.FillPath(&brush, &path);
    }

    if (p.getStrokeOpacity() > 0.f && p.getStrokeWidth() > 0.f) {
        Color color(
            (BYTE)(p.getStrokeOpacity() * 255),
            p.getStroke().GetR(),
            p.getStroke().GetG(),
            p.getStroke().GetB()
        );
        Pen pen(color, p.getStrokeWidth());
        g.DrawPath(&pen, &path);
    }

    g.Restore(state);
}


void Renderer::render(const SvgGroup& grp) {
    GraphicsState state = g.Save();

    applyTransform(g, grp.getTransform());

    const auto& elements = grp.getElements();
    for (const auto& element : elements) {
        if (element) {
            element->accept(*this);
        }
    }

    g.Restore(state);
}