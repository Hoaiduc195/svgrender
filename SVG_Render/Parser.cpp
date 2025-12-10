#pragma once
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
#include <regex>
#include <sstream>

//Ham phan tich chuoi Transform
static void parseTransformAttribute(const string& transformStr, SvgElement* element) {
    if (transformStr.empty()) return;

    //Lay ban sao Transform hien tai
    Transform currentTrans = element->getTransform();

    regex re("([a-z]+)\\(([^)]+)\\)");
    auto words_begin = sregex_iterator(transformStr.begin(), transformStr.end(), re);
    auto words_end = sregex_iterator();

    for (sregex_iterator i = words_begin; i != words_end; ++i) {
        smatch match = *i;
        string command = match.str(1);
        string args = match.str(2);

        //Replace ',' = space ' ' to parse easily
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
    }

    //set nguoc vao doi tuong
    element->setTransform(currentTrans);
}
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
    if (value.empty() || value == "none") return Color(0, 0, 0, 0);

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

// New recursive function return unique_ptr<SvgElement> instead of add directly to Doc
// `parentStyle` use to ... <g> -> child
unique_ptr<SvgElement> Parser::parseElementRecursive(tinyxml2::XMLElement* element, const SvgElement* parent) {
    if (!element) return nullptr;

    string tag = element->Name();
    unique_ptr<SvgElement> svgObj = nullptr;
    if (tag == "g") { 
        auto group = make_unique<SvgGroup>();
        svgObj = move(group);
    }
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
        svgObj = make_unique<SvgLine>(x1,y1,x2,y2);
    }
    else if (tag == "ellipse") {
        float cx = element->FloatAttribute("cx", 0);
        float cy = element->FloatAttribute("cy", 0);
        float rx = element->FloatAttribute("rx", 0);
        float ry = element->FloatAttribute("ry", 0);
        svgObj = make_unique<SvgEllipse>(cx,cy,rx,ry);
    }
    else if (tag == "polygon") {
        const char* pointsAttr = element->Attribute("points");
        vector<Vector2> pts;
        if (pointsAttr) {
            pts = parsePoints(pointsAttr);
        }
        //khep kin
        svgObj = make_unique<SvgPolygon>(pts, true);
    }
    else if (tag == "polyline") {
        const char* pointsAttr = element->Attribute("points");
        vector<Vector2> pts;
        if (pointsAttr) {
            pts = parsePoints(pointsAttr);
        }
        svgObj = make_unique<SvgPolyline>(pts, false); //kh khep kin
    }
    else if (tag == "text") {
        float x = element->FloatAttribute("x", 0);
        float y = element->FloatAttribute("y", 0);
        float fontSize = element->FloatAttribute("font-size", 12.0f);
        string content = element->GetText();
        svgObj = make_unique<SvgText>(x, y, fontSize, content);
    }
    if (!svgObj) return nullptr;

    //Fill
    const char* fillAttr = element->Attribute("fill");
    if (fillAttr) {
        svgObj->setFill(parseColor(fillAttr));
    } 
    else if (parent) {
        svgObj->setFill(parent->getFill());
    }

    //stroke
    const char* strokeAttr = element->Attribute("stroke");
    if (strokeAttr){
        svgObj->setStroke(parseColor(strokeAttr));
    }
    else if (parent) {
        svgObj->setStroke(parent->getStroke());
    }

    // Stroke Width
    if (element->Attribute("stroke-width")) {
        svgObj->setStrokeWidth(element->FloatAttribute("stroke-width"));
    }
    else if (parent) {
        svgObj->setStrokeWidth(parent->getStrokeWidth());
    }

    // Stroke Opacity
    if (element->Attribute("stroke-opacity")) {
        svgObj->setStrokeOpacity(element->FloatAttribute("stroke-opacity"));
    }
    else if (parent) {
        svgObj->setStrokeOpacity(parent->getStrokeOpacity());
    }

    // Fill Opacity
    if (element->Attribute("fill-opacity")) {
        svgObj->setFillOpacity(element->FloatAttribute("fill-opacity"));
    }
    else if (parent) {
        svgObj->setFillOpacity(parent->getFillOpacity());
    }

    // Transform
    // Transform khong ke thua theo kieu copy gia tri nhu color
    // Transform của cha tac dong len con thong qua cau truc long nhau Group
    // nen chi parse transform cua the nay thoi
    const char* transformAttr = element->Attribute("transform");
    if (transformAttr) {
        parseTransformAttribute(transformAttr, svgObj.get());
    }

    //Neu la group, tiep tuc de quy
    if (tag == "g") {
        SvgGroup* groupPtr = static_cast<SvgGroup*>(svgObj.get());
        tinyxml2::XMLElement* child = element->FirstChildElement();
        while (child) {
            // Truyền `groupPtr` xuống làm `parent` cho các con
            auto childObj = parseElementRecursive(child, groupPtr);
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
            // Truyền defaultContext vào làm cha
            auto obj = parseElementRecursive(child, &defaultContext);
            if (obj) {
                doc.addElement(move(obj));
            }
            child = child->NextSiblingElement();
        }
    }
    else {
        // Trường hợp file SVG không có thẻ <svg> bao ngoài (ít gặp)
        auto obj = parseElementRecursive(element, &defaultContext);
        if (obj) {
            doc.addElement(move(obj));
        }
    }
}

unique_ptr<SvgDocument> Parser::parseSVG(const string& xmlText) {
    auto svgDoc = make_unique<SvgDocument>(); //chua danh sach cac phan tu svg

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

    parseElement(root, *svgDoc);//duyet cay tu <svg> xuong de them cac doi tuong vo doc
    return svgDoc;
}