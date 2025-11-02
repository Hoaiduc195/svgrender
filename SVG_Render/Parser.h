#pragma once
#include "tinyxml2.h"
#include "SvgDocument.h"
#include "SvgElement.h"
#include "SvgCircle.h"
#include "SvgRect.h"
#include "gdipluscolor.h"
//#include <memory>
#include <iostream>
#include <string>

using namespace std;
using namespace Gdiplus;

class Parser {
public:
    static unique_ptr<SvgDocument> parseSVG(const string& xmlText) {
        tinyxml2::XMLDocument doc;
        doc.Parse(xmlText.c_str());

        auto svg = make_unique<SvgDocument>();
        tinyxml2::XMLElement* root = doc.FirstChildElement("svg");
        if (!root) return svg;

        for (tinyxml2::XMLElement* elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement()) {
            string name = elem->Name();

            if (name == "rect") {
                float x = elem->FloatAttribute("x");
                float y = elem->FloatAttribute("y");
                float w = elem->FloatAttribute("width");
                float h = elem->FloatAttribute("height");
                const char* fill = elem->Attribute("fill");

                Color color;
                if (fill && fill[0] == '#') {
                    unsigned int rgb = stoul(fill + 1, nullptr, 16);
                    BYTE r = (rgb >> 16) & 0xFF;
                    BYTE g = (rgb >> 8) & 0xFF;
                    BYTE b = rgb & 0xFF;
                    ARGB argb = Color::MakeARGB(255, r, g, b);
                    color = Color(argb);
                }

                svg->addChild(make_unique<SvgRect>(x, y, w, h, color));
            }
            else if (name == "circle") {
                float cx = elem->FloatAttribute("cx");
                float cy = elem->FloatAttribute("cy");
                float r = elem->FloatAttribute("r");
                const char* fill = elem->Attribute("fill");

                Color color;
                if (fill && fill[0] == '#') {
                    unsigned int rgb = stoul(fill + 1, nullptr, 16);
                    BYTE r = (rgb >> 16) & 0xFF;
                    BYTE g = (rgb >> 8) & 0xFF;
                    BYTE b = rgb & 0xFF;
                    ARGB argb = Color::MakeARGB(255, r, g, b);
                    color = Color(argb);
                }

                svg->addChild(make_unique<SvgCircle>(cx, cy, r, color));
            }
        }

        return svg;
    }
};
