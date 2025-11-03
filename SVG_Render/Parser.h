#pragma once
#include "SvgDocument.h"
#include "SvgElement.h"
#include "framework.h"
#include "tinyxml2.h"


class Parser {
public:
    static unique_ptr<SvgDocument> parseSVG(const string& xmlText);
private:
    static void parseElement(tinyxml2::XMLElement* element, SvgDocument& doc);
    static Color parseColor(const string& value);
};
