#pragma once
#include "Parser.h"
#include "SvgCircle.h"
#include "SvgRect.h"
#include "SvgLine.h"
#include "SvgEllipse.h"
#include "SvgPolygon.h"
#include "SvgPolyline.h"
#include "SvgText.h"

static vector<Vector2> parsePoints(const string& pointsStr) {
    vector<Vector2> pts;
    stringstream ss(pointsStr);
    string token;

    while (getline(ss, token, ' ')) {
        size_t commaPos = token.find(',');
        if (commaPos != string::npos) {
            try {
                float x = stof(token.substr(0, commaPos));
                float y = stof(token.substr(commaPos + 1));
                pts.emplace_back(x, y);
            }
            catch (...) { //doc kh duoc thi bo qua 
                continue;
            }
        }
    }
    return pts;
}

Color Parser::parseColor(const string& value) {
    if (value.empty()) return Color::Black;

    unsigned int r, g, b;
    if (sscanf_s(value.c_str(), "rgb(%d,%d,%d)", &r, &g, &b) == 3)
       return Color(255, r, g, b);

    //neu dau vao la ten
    if (value == "red") return Color::Red;
    if (value == "blue") return Color::Blue;
    if (value == "green") return Color::Green;
    if (value == "black") return Color::Black;
    if (value == "yellow") return Color::Yellow;
    if (value == "white") return Color::White;

    return Color::Black;
}

void Parser::parseElement(tinyxml2::XMLElement* element, SvgDocument& doc) {
    if (!element) return;

    string tag = element->Name();

    if (tag == "rect") {
        float x = element->FloatAttribute("x", 0);
        float y = element->FloatAttribute("y", 0);
        float w = element->FloatAttribute("width", 0);
        float h = element->FloatAttribute("height", 0);
        float rx = element->FloatAttribute("rx", 0);
        float ry = element->FloatAttribute("ry", 0);
        auto rect = make_unique<SvgRect>(x, y, w, h, rx, ry);

        string fillStr = element->Attribute("fill") ? element->Attribute("fill") : "";
        Color fill = parseColor(fillStr);
        rect->setFill(fill);

        string strokeStr = element->Attribute("stroke") ? element->Attribute("stroke") : "";
        Color stroke = parseColor(strokeStr);
        rect->setStroke(stroke);

        float strokeWidth = element->FloatAttribute("stroke-width", 0);
        rect->setStrokeWidth(strokeWidth);

        float strokeOpacity = element->FloatAttribute("stroke-opacity", 1);
        rect->setStrokeOpacity(strokeOpacity);

        float fillOpacity = element->FloatAttribute("fill-opacity", 1);
        rect->setFillOpacity(fillOpacity);

        //them vao tai lieu svg
        doc.addElement(move(rect));
    }
    else if (tag == "circle") {
        float cx = element->FloatAttribute("cx", 0);
        float cy = element->FloatAttribute("cy", 0);
        float r = element->FloatAttribute("r", 0);
        auto cir = make_unique<SvgCircle>(cx, cy, r);

        string fillStr = element->Attribute("fill") ? element->Attribute("fill") : "";
        Color fill = parseColor(fillStr);
        cir->setFill(fill);

        string strokeStr = element->Attribute("stroke") ? element->Attribute("stroke") : "";
        Color stroke = parseColor(strokeStr);
        cir->setStroke(stroke);

        float strokeWidth = element->FloatAttribute("stroke-width", 0);
        cir->setStrokeWidth(strokeWidth);

        float strokeOpacity = element->FloatAttribute("stroke-opacity", 1);
        cir->setStrokeOpacity(strokeOpacity);

        float fillOpacity = element->FloatAttribute("fill-opacity", 1);
        cir->setFillOpacity(fillOpacity);
        doc.addElement(move(cir));
    }
    else if (tag == "line") {
        float x1 = element->FloatAttribute("x1", 0);
        float y1 = element->FloatAttribute("y1", 0);
        float x2 = element->FloatAttribute("x2", 0);
        float y2 = element->FloatAttribute("y2", 0);
        auto line = make_unique<SvgLine>(x1,y1,x2,y2);

        string fillStr = element->Attribute("fill") ? element->Attribute("fill") : "";
        Color fill = parseColor(fillStr);
        line->setFill(fill);

        string strokeStr = element->Attribute("stroke") ? element->Attribute("stroke") : "";
        Color stroke = parseColor(strokeStr);
        line->setStroke(stroke);

        float strokeWidth = element->FloatAttribute("stroke-width", 0);
        line->setStrokeWidth(strokeWidth);

        float strokeOpacity = element->FloatAttribute("stroke-opacity", 1);
        line->setStrokeOpacity(strokeOpacity);

        float fillOpacity = element->FloatAttribute("fill-opacity", 1);
        line->setFillOpacity(fillOpacity);
        doc.addElement(move(line));

    }
    else if (tag == "ellipse") {
        float cx = element->FloatAttribute("cx", 0);
        float cy = element->FloatAttribute("cy", 0);
        float rx = element->FloatAttribute("rx", 0);
        float ry = element->FloatAttribute("ry", 0);
        
        auto elli = make_unique<SvgEllipse>(cx,cy,rx,ry);

        string fillStr = element->Attribute("fill") ? element->Attribute("fill") : "";
        Color fill = parseColor(fillStr);
        elli->setFill(fill);

        string strokeStr = element->Attribute("stroke") ? element->Attribute("stroke") : "";
        Color stroke = parseColor(strokeStr);
        elli->setStroke(stroke);

        float strokeWidth = element->FloatAttribute("stroke-width", 0);
        elli->setStrokeWidth(strokeWidth);

        float strokeOpacity = element->FloatAttribute("stroke-opacity", 1);
        elli->setStrokeOpacity(strokeOpacity);

        float fillOpacity = element->FloatAttribute("fill-opacity", 1);
        elli->setFillOpacity(fillOpacity);
        doc.addElement(move(elli));
    }

    else if (tag == "polygon") {
        const char* pointsAttr = element->Attribute("points");
        vector<Vector2> pts;
        if (pointsAttr) {
            pts = parsePoints(pointsAttr);
        }

        //khep kin
        auto poly = make_unique<SvgPolygon>(pts, true);

        string fillStr = element->Attribute("fill") ? element->Attribute("fill") : "";
        Color fill = parseColor(fillStr);
        poly->setFill(fill);

        string strokeStr = element->Attribute("stroke") ? element->Attribute("stroke") : "";
        Color stroke = parseColor(strokeStr);
        poly->setStroke(stroke);

        float strokeWidth = element->FloatAttribute("stroke-width", 0);
        poly->setStrokeWidth(strokeWidth);

        float strokeOpacity = element->FloatAttribute("stroke-opacity", 1);
        poly->setStrokeOpacity(strokeOpacity);

        float fillOpacity = element->FloatAttribute("fill-opacity", 1);
        poly->setFillOpacity(fillOpacity);

        doc.addElement(move(poly));
    }
    else if (tag == "polyline") {
        const char* pointsAttr = element->Attribute("points");
        vector<Vector2> pts;
        if (pointsAttr) {
            pts = parsePoints(pointsAttr);
        }

        auto polyline = make_unique<SvgPolyline>(pts, false); //kh khep kin

        string fillStr = element->Attribute("fill") ? element->Attribute("fill") : "";
        Color fill = parseColor(fillStr);
        polyline->setFill(fill);

        string strokeStr = element->Attribute("stroke") ? element->Attribute("stroke") : "";
        Color stroke = parseColor(strokeStr);
        polyline->setStroke(stroke);

        float strokeWidth = element->FloatAttribute("stroke-width", 0);
        polyline->setStrokeWidth(strokeWidth);

        float strokeOpacity = element->FloatAttribute("stroke-opacity", 1);
        polyline->setStrokeOpacity(strokeOpacity);

        float fillOpacity = element->FloatAttribute("fill-opacity", 1);
        polyline->setFillOpacity(fillOpacity);

        doc.addElement(move(polyline));
    }
    else if (tag == "text") {
        float x = element->FloatAttribute("x", 0);
        float y = element->FloatAttribute("y", 0);
        float fontSize = element->FloatAttribute("font-size", 12.0f);
        string content = element->GetText();

        auto text = make_unique<SvgText>(x, y, fontSize, content);

        string fillStr = element->Attribute("fill") ? element->Attribute("fill") : "";
        if (!fillStr.empty()) {
            Color fill = parseColor(fillStr);
            text->setFill(fill);
        }

        string strokeStr = element->Attribute("stroke") ? element->Attribute("stroke") : "";
        if (!strokeStr.empty()) {
            Color stroke = parseColor(strokeStr);
            text->setStroke(stroke);
        }

        float strokeWidth = element->FloatAttribute("stroke-width", 0);
        text->setStrokeWidth(strokeWidth);

        float strokeOpacity = element->FloatAttribute("stroke-opacity", 1);
        text->setStrokeOpacity(strokeOpacity);

        float fillOpacity = element->FloatAttribute("fill-opacity", 1);
        text->setFillOpacity(fillOpacity);

        doc.addElement(move(text));
        }
    else if (tag == "svg") { // optional
        tinyxml2::XMLElement* child = element->FirstChildElement();
        while (child) {
            parseElement(child, doc);
            child = child->NextSiblingElement();
        }
    }
}

unique_ptr<SvgDocument> Parser::parseSVG(const string& xmlText) {
    auto svgDoc = make_unique<SvgDocument>();

    tinyxml2::XMLDocument doc;
    if (doc.Parse(xmlText.c_str()) != tinyxml2::XML_SUCCESS) {
        cerr << "Parse error!\n";
        return nullptr;
    }

    tinyxml2::XMLElement* root = doc.RootElement();
    if (!root || string(root->Name()) != "svg") {
        cerr << "Not a valid SVG file.\n";
        return nullptr;
    }

    parseElement(root, *svgDoc);
    return svgDoc;
}