#pragma once
#include "SvgElement.h"
#include "framework.h"

class SvgText : public SvgElement {
private:
    float x, y;         
    float fontSize;    
    string content;     

public:
    SvgText();
    SvgText(const SvgText& other);
    SvgText(float _x, float _y, float _fontSize, const string& _content);
    ~SvgText();

    void draw(Graphics& g) override;

	// Setter
    void setFontSize(float size);
};
