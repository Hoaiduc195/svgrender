#include "Parser.h"
#include "SvgCircle.h"
#include "SvgRect.h"
#include "SvgLine.h"
#include "SvgEllipse.h"
#include "SvgPolygon.h"
#include "SvgPolyline.h"
#include "SvgText.h"
#include "SvgPath.h"
#include "SvgGroup.h"
#include "Transform.h" 
#include <regex>
#include <sstream>
#include <algorithm>
#include <string>

// Helper to safely clamp values
template <typename T>
T clamp(T val, T minVal, T maxVal) {
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

// --- Helper Functions ---

// NEW: Helper to parse attributes like "50%" -> 0.5f or "0.5" -> 0.5f
float parseCoordinate(const char* str, float defaultVal) {
    if (!str) return defaultVal;
    std::string s = str;
    if (s.empty()) return defaultVal;

    try {
        float val = std::stof(s);
        if (s.back() == '%') {
            return val / 100.0f;
        }
        return val;
    }
    catch (...) {
        return defaultVal;
    }
}

static void parseTransformAttribute(const string& transformStr, SvgElement* element) {
    if (transformStr.empty()) return;
    Transform currentTrans = element->getTransform();
    regex re("([a-z]+)\\s*\\(([^)]+)\\)");
    auto words_begin = sregex_iterator(transformStr.begin(), transformStr.end(), re);
    auto words_end = sregex_iterator();

    for (sregex_iterator i = words_begin; i != words_end; ++i) {
        smatch match = *i;
        string command = match.str(1);
        string args = match.str(2);
        replace(args.begin(), args.end(), ',', ' ');
        stringstream ss(args);
        float val1 = 0, val2 = 0;

        if (command == "translate") {
            ss >> val1;
            if (!(ss >> val2)) val2 = 0;
            currentTrans.translate(val1, val2);
        }
        else if (command == "rotate") {
            ss >> val1;
            currentTrans.rotate(val1);
        }
        else if (command == "scale") {
            ss >> val1;
            if (!(ss >> val2)) val2 = val1;
            currentTrans.scale(val1, val2);
        }
        else if (command == "matrix") {
            double a, b, c, d, e, f;
            if (ss >> a >> b >> c >> d >> e >> f) {
                currentTrans.matrix(a, b, c, d, e, f);
            }
        }
    }
    element->setTransform(currentTrans);
}

static vector<Vector2> parsePoints(const string& pointsStr) {
    vector<Vector2> pts;
    string cleanStr = pointsStr;
    replace(cleanStr.begin(), cleanStr.end(), ',', ' ');
    stringstream ss(cleanStr);
    float x, y;
    while (ss >> x >> y) {
        pts.emplace_back(x, y);
    }
    return pts;
}

// --- Parser Class Implementation ---

Color Parser::parseColor(const string& value) {
    if (value.empty() || value == "none") return Color(0, 0, 0, 0);
    if (value.find("url(") != string::npos) return Color::Black;

    if (value[0] == '#') {
        string hexStr = value.substr(1);
        unsigned long val = strtoul(hexStr.c_str(), nullptr, 16);
        if (hexStr.length() == 6) {
            return Color(255, (val >> 16) & 0xFF, (val >> 8) & 0xFF, val & 0xFF);
        }
        else if (hexStr.length() == 3) {
            unsigned int r = (val >> 8) & 0xF;
            unsigned int g = (val >> 4) & 0xF;
            unsigned int b = val & 0xF;
            return Color(255, r * 17, g * 17, b * 17);
        }
    }

    static const regex re(R"(rgb\s*\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\))");
    smatch match;
    if (regex_search(value, match, re)) {
        int r = stoi(match[1]);
        int g = stoi(match[2]);
        int b = stoi(match[3]);
        return Color(255, clamp(r, 0, 255), clamp(g, 0, 255), clamp(b, 0, 255));
    }

    return getColorByName(value);
}

unique_ptr<Gradient> Parser::parseGradient(tinyxml2::XMLElement* elem, SvgDocument* doc) {
    string tag = elem->Name();
    unique_ptr<Gradient> grad = nullptr;

    // 1. Determine Type & Default Coordinates (Using parseCoordinate)
    if (tag == "linearGradient") {
        auto lGrad = make_unique<LinearGradient>();
        lGrad->x1 = parseCoordinate(elem->Attribute("x1"), 0.0f);
        lGrad->y1 = parseCoordinate(elem->Attribute("y1"), 0.0f);
        lGrad->x2 = parseCoordinate(elem->Attribute("x2"), 1.0f);
        lGrad->y2 = parseCoordinate(elem->Attribute("y2"), 0.0f);
        grad = move(lGrad);
    }
    else if (tag == "radialGradient") {
        auto rGrad = make_unique<RadialGradient>();
        rGrad->cx = parseCoordinate(elem->Attribute("cx"), 0.5f);
        rGrad->cy = parseCoordinate(elem->Attribute("cy"), 0.5f);
        rGrad->r = parseCoordinate(elem->Attribute("r"), 0.5f);
        rGrad->fx = parseCoordinate(elem->Attribute("fx"), rGrad->cx);
        rGrad->fy = parseCoordinate(elem->Attribute("fy"), rGrad->cy);
        grad = move(rGrad);
    }

    if (!grad) return nullptr;

    // 2. ID
    grad->id = elem->Attribute("id") ? elem->Attribute("id") : "";

    // 3. Units
    const char* unitsAttr = elem->Attribute("gradientUnits");
    if (unitsAttr && string(unitsAttr) == "userSpaceOnUse") {
        grad->units = GradientUnits::UserSpaceOnUse;
    }
    else {
        grad->units = GradientUnits::ObjectBoundingBox;
    }

    // 4. Transform (gradientTransform)
    const char* transAttr = elem->Attribute("gradientTransform");
    if (transAttr) {
        SvgGroup dummy;
        parseTransformAttribute(transAttr, &dummy);
        grad->transform = dummy.getTransform();
    }

    // 5. Inheritance (xlink:href)
    const char* href = elem->Attribute("xlink:href");
    if (!href) href = elem->Attribute("href");

    if (href && doc) {
        string linkId = href;
        if (linkId.size() > 1 && linkId[0] == '#') linkId = linkId.substr(1);
        const Gradient* parent = doc->getGradient(linkId);
        if (parent) {
            grad->stops = parent->stops;
        }
    }

    // 6. Parse Stops
    vector<GradientStop> ownStops;
    tinyxml2::XMLElement* child = elem->FirstChildElement();
    while (child) {
        if (string(child->Name()) == "stop") {
            GradientStop stop;
            // Use helper for offset (handles %)
            stop.offset = parseCoordinate(child->Attribute("offset"), 0.0f);
            stop.offset = clamp(stop.offset, 0.0f, 1.0f);

            const char* colorAttr = child->Attribute("stop-color");
            float stopOpacity = child->FloatAttribute("stop-opacity", 1.0f);

            Color c = colorAttr ? Parser::parseColor(colorAttr) : Color::Black;
            stop.color = Color((unsigned char)(stopOpacity * 255), c.GetR(), c.GetG(), c.GetB());

            ownStops.push_back(stop);
        }
        child = child->NextSiblingElement();
    }

    if (!ownStops.empty()) {
        grad->stops = ownStops;
    }

    return grad;
}

unique_ptr<SvgElement> Parser::parseElementRecursive(tinyxml2::XMLElement* element, const SvgElement* parent, SvgDocument* doc) {
    if (!element) return nullptr;
    string tag = element->Name();

    // Definitions
    if (tag == "defs") {
        tinyxml2::XMLElement* child = element->FirstChildElement();
        while (child) {
            string childTag = child->Name();
            if (childTag == "linearGradient" || childTag == "radialGradient") {
                auto grad = parseGradient(child, doc);
                if (grad && doc) doc->addGradient(move(grad));
            }
            child = child->NextSiblingElement();
        }
        return nullptr;
    }

    // Standalone Gradient
    if (tag == "linearGradient" || tag == "radialGradient") {
        auto grad = parseGradient(element, doc);
        if (grad && doc) doc->addGradient(move(grad));
        return nullptr;
    }

    unique_ptr<SvgElement> svgObj = nullptr;

    if (tag == "g") svgObj = make_unique<SvgGroup>();
    else if (tag == "path") {
        string d = element->Attribute("d") ? element->Attribute("d") : "";
        svgObj = make_unique<SvgPath>(d);
    }
    else if (tag == "rect") {
        float x = element->FloatAttribute("x", 0);
        float y = element->FloatAttribute("y", 0);
        float w = element->FloatAttribute("width", 0);
        float h = element->FloatAttribute("height", 0);
        float rx = element->FloatAttribute("rx", 0);
        float ry = element->FloatAttribute("ry", 0);
        svgObj = make_unique<SvgRect>(x, y, w, h, rx, ry);
    }
    else if (tag == "circle") {
        float cx = element->FloatAttribute("cx", 0);
        float cy = element->FloatAttribute("cy", 0);
        float r = element->FloatAttribute("r", 0);
        svgObj = make_unique<SvgCircle>(cx, cy, r);
    }
    else if (tag == "line") {
        float x1 = element->FloatAttribute("x1", 0);
        float y1 = element->FloatAttribute("y1", 0);
        float x2 = element->FloatAttribute("x2", 0);
        float y2 = element->FloatAttribute("y2", 0);
        svgObj = make_unique<SvgLine>(x1, y1, x2, y2);
    }
    else if (tag == "ellipse") {
        float cx = element->FloatAttribute("cx", 0);
        float cy = element->FloatAttribute("cy", 0);
        float rx = element->FloatAttribute("rx", 0);
        float ry = element->FloatAttribute("ry", 0);
        svgObj = make_unique<SvgEllipse>(cx, cy, rx, ry);
    }
    else if (tag == "polygon") {
        const char* pointsAttr = element->Attribute("points");
        vector<Vector2> pts;
        if (pointsAttr) pts = parsePoints(pointsAttr);
        svgObj = make_unique<SvgPolygon>(pts, true);
    }
    else if (tag == "polyline") {
        const char* pointsAttr = element->Attribute("points");
        vector<Vector2> pts;
        if (pointsAttr) pts = parsePoints(pointsAttr);
        svgObj = make_unique<SvgPolyline>(pts, false);
    }
    else if (tag == "text") {
        float x = element->FloatAttribute("x", 0);
        float y = element->FloatAttribute("y", 0);
        float dx = element->FloatAttribute("dx", 0);
        float dy = element->FloatAttribute("dy", 0);
        x += dx; y += dy;
        float fontSize = element->FloatAttribute("font-size", 12.0f);
        const char* txt = element->GetText();
        string content = txt ? txt : "";
        const char* textAnchorAttr = element->Attribute("text-anchor");
        string textAnchor = textAnchorAttr ? textAnchorAttr : "start";
        svgObj = make_unique<SvgText>(x, y, fontSize, content, textAnchor);
    }

    if (!svgObj) return nullptr;

    const char* fillAttr = element->Attribute("fill");
    if (fillAttr) {
        string fillStr = fillAttr;
        if (fillStr.find("url(#") != string::npos) {
            size_t start = fillStr.find("#") + 1;
            size_t end = fillStr.find(")");
            string id = fillStr.substr(start, end - start);
            svgObj->setFillGradient(id);
        }
        else {
            svgObj->setFill(parseColor(fillAttr));
            if (fillStr == "none") {
                svgObj->setFillOpacity(0.0f);
            }
        }
    }
    else if (parent) {
        if (parent->getFillType() == FillType::Gradient) {
            svgObj->setFillGradient(parent->getGradientId());
        }
        else {
            svgObj->setFill(parent->getFill());
        }
        svgObj->setFillOpacity(parent->getFillOpacity());
    }

    if (element->Attribute("fill-opacity")) {
        svgObj->setFillOpacity(element->FloatAttribute("fill-opacity"));
    }

    const char* strokeAttr = element->Attribute("stroke");
    if (strokeAttr) {
        svgObj->setStroke(parseColor(strokeAttr));
        if (string(strokeAttr) == "none") {
            svgObj->setStrokeOpacity(0.0f);
        }
        else {
            svgObj->setStrokeOpacity(1.0f);
        }
    }
    else if (parent) {
        svgObj->setStroke(parent->getStroke());
        svgObj->setStrokeOpacity(parent->getStrokeOpacity());
    }

    if (element->Attribute("stroke-opacity")) {
        svgObj->setStrokeOpacity(element->FloatAttribute("stroke-opacity"));
    }
    if (element->Attribute("stroke-width")) {
        svgObj->setStrokeWidth(element->FloatAttribute("stroke-width"));
    }
    else if (parent) {
        svgObj->setStrokeWidth(parent->getStrokeWidth());
    }

    const char* transformAttr = element->Attribute("transform");
    if (transformAttr) {
        parseTransformAttribute(transformAttr, svgObj.get());
    }

    if (tag == "g") {
        SvgGroup* groupPtr = static_cast<SvgGroup*>(svgObj.get());
        tinyxml2::XMLElement* child = element->FirstChildElement();
        while (child) {
            auto childObj = parseElementRecursive(child, groupPtr, doc);
            if (childObj) {
                groupPtr->addElement(move(childObj));
            }
            child = child->NextSiblingElement();
        }
    }

    return svgObj;
}

void Parser::parseElement(tinyxml2::XMLElement* element, SvgDocument& doc) {
    SvgGroup defaultContext;

    if (string(element->Name()) == "svg") {
        tinyxml2::XMLElement* child = element->FirstChildElement();
        while (child) {
            auto obj = parseElementRecursive(child, &defaultContext, &doc);
            if (obj) {
                doc.addElement(move(obj));
            }
            child = child->NextSiblingElement();
        }
    }
    else {
        auto obj = parseElementRecursive(element, &defaultContext, &doc);
        if (obj) {
            doc.addElement(move(obj));
        }
    }
}

unique_ptr<SvgDocument> Parser::parseSVG(const string& xmlText) {
    auto svgDoc = make_unique<SvgDocument>();
    tinyxml2::XMLDocument doc;
    if (doc.Parse(xmlText.c_str()) != tinyxml2::XML_SUCCESS) {
        return nullptr;
    }

    tinyxml2::XMLElement* root = doc.RootElement();
    if (!root) return nullptr;

    parseElement(root, *svgDoc);
    return svgDoc;
}