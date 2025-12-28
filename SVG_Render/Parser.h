#pragma once
#include "SvgDocument.h"
#include "SvgElement.h"
#include "Color.h"
#include "Gradient.h" 
#include "framework.h"
#include "tinyxml2.h"
#include <string>
#include <memory>

class Parser {
public:
    static std::unique_ptr<SvgDocument> parseSVG(const std::string& xmlText);

private:
    static std::unique_ptr<SvgElement> parseElementRecursive(
        tinyxml2::XMLElement* element,
        const SvgElement* parent,
        SvgDocument* doc);

    static std::unique_ptr<Gradient> parseGradient(
        tinyxml2::XMLElement* elem,
        SvgDocument* doc
    );

    static void parseElement(tinyxml2::XMLElement* element, SvgDocument& doc);
    static Color parseColor(const std::string& value);
};