#pragma once
#include "SvgDocument.h"
#include "SvgElement.h"
#include "Color.h"
#include "framework.h"
#include "tinyxml2.h"
#include <map>


class Parser {
public:
    static unique_ptr<SvgDocument> parseSVG(const string& xmlText);
private:
    static unique_ptr<SvgElement> parseElementRecursive(
        tinyxml2::XMLElement* element,
        const SvgElement* parent,
        std::map<string, LinearGradient>& declaredGradients);
    static LinearGradient parseLinearGradient(tinyxml2::XMLElement* elem);
    static void parseElement(tinyxml2::XMLElement* element, SvgDocument& doc);
    static Color parseColor(const string& value);
};
