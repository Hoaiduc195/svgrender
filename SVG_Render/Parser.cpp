#include "Parser.h"
#include "SvgCircle.h"
#include "SvgRect.h"
#include "SvgLine.h"
#include "SvgEllipse.h"
#include "SvgPolygon.h"

Color Parser::parseColor(const string& value) {
    if (value.empty()) return Color::Black;

    if (value[0] == '#') {
        unsigned int r, g, b;
        if (sscanf_s(value.c_str() + 1, "%02x%02x%02x", &r, &g, &b) == 3)
            return Color(255, r, g, b);
    }

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
        string fillStr = element->Attribute("fill") ? element->Attribute("fill") : "";
        Color fill = parseColor(fillStr);
		auto rect = make_unique<SvgRect>(x, y, w, h, rx, ry);
		// TODO: Update fill, stroke, etc. properties
		// rect->setFill(fill); ...
        doc.addElement(move(rect));
    }
    else if (tag == "circle") {
        float cx = element->FloatAttribute("cx", 0);
        float cy = element->FloatAttribute("cy", 0);
        float r = element->FloatAttribute("r", 0);
        /*string fillStr = element->Attribute("fill") ? element->Attribute("fill") : "";
        Color fill = parseColor(fillStr);*/

        // TODO: Update fill, stroke, etc. properties
        doc.addElement(make_unique<SvgCircle>(cx, cy, r));
    }
    else if (tag == "line") {
        float x1 = element->FloatAttribute("x1", 0);
        float y1 = element->FloatAttribute("y1", 0);
        float x2 = element->FloatAttribute("x2", 0);
        float y2 = element->FloatAttribute("y2", 0);
       /* string fillStr = element->Attribute("fill") ? element->Attribute("fill") : "";
        Color fill = parseColor(fillStr);*/
            
        // TODO: Update fill, stroke, etc. properties
        doc.addElement(make_unique<SvgLine>(x1, x2, y1, y2));
    }
    else if (tag == "ellipse") {
        float cx = element->FloatAttribute("cx", 0);
        float cy = element->FloatAttribute("cy", 0);
        float rx = element->FloatAttribute("rx", 0);
        float ry = element->FloatAttribute("ry", 0);
        /* string fillStr = element->Attribute("fill") ? element->Attribute("fill") : "";
         Color fill = parseColor(fillStr);*/

        // TODO: Update fill, stroke, etc. properties
        doc.addElement(make_unique<SvgEllipse>(cx, cy, rx, ry));
    }
    //thieu polygon,polyline
    else if (tag == "svg") {
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