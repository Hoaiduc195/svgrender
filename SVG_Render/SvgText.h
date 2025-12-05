#pragma once
#include "SvgElement.h"
#include "framework.h"

class Renderer;

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

    void accept(Renderer& renderer) override;

    // Setter
    void setFontSize(float size);

    // Getters for renderer
    float getX() const { return x; }
    float getY() const { return y; }
    float getFontSize() const { return fontSize; }
    const string& getContent() const { return content; }
};
