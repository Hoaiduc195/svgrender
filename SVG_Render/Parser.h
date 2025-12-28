#pragma once
#include "SvgDocument.h"
#include "SvgElement.h"
#include "Color.h"
#include "Gradient.h" 
#include "framework.h"
#include "tinyxml2.h"


class Parser {
public:
    static unique_ptr<SvgDocument> parseSVG(const string& xmlText);

private:
    static unique_ptr<SvgElement> parseElementRecursive(
        tinyxml2::XMLElement* element,
        const SvgElement* parent,
        SvgDocument* doc);

    static unique_ptr<Gradient> parseGradient(tinyxml2::XMLElement* elem);

    static void parseElement(tinyxml2::XMLElement* element, SvgDocument& doc);
    static Color parseColor(const string& value);
};