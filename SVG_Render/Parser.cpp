#include "Parser.h"

Color Parser::parseColor(const string& value) {
    if (value.empty()) return Color::Black;

    if (value[0] == '#') {
        unsigned int r, g, b;
        if (sscanf(value.c_str() + 1, "%02x%02x%02x", &r, &g, &b) == 3)
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

void Parser::ParseElement(tinyxml2::XMLElement* element, SvgDocument& doc) {
    if (!element) return;

    string tag = element->Name();

    if (tag == "rect") {
        float x = element->FloatAttribute("x", 0);
        float y = element->FloatAttribute("y", 0);
        float w = element->FloatAttribute("width", 0);
        float h = element->FloatAttribute("height", 0);
        string fillStr = element->Attribute("fill") ? element->Attribute("fill") : "";
        Color fill = parseColor(fillStr);

        doc.addElement(make_unique<SvgRect>(x, y, w, h, fill));
    }
    else if (tag == "circle") {
        float cx = element->FloatAttribute("cx", 0);
        float cy = element->FloatAttribute("cy", 0);
        float r = element->FloatAttribute("r", 0);
        string fillStr = element->Attribute("fill") ? element->Attribute("fill") : "";
        Color fill = parseColor(fillStr);

        doc.addElement(make_unique<SvgCircle>(cx, cy, r, fill));
    }
    else if (tag == "svg") {
        tinyxml2::XMLElement* child = element->FirstChildElement();
        while (child) {
            ParseElement(child, doc);
            child = child->NextSiblingElement();
        }
    }
    else {
       //the line, path...
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

    ParseElement(root, *svgDoc);
    return svgDoc;
}