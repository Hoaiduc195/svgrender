#pragma once
#include "SvgElement.h"
#include "framework.h"

class Renderer;

class SvgText : public SvgElement {
private:
    float x, y;         
    float fontSize;    
    string content;  
    string textAnchor;


public:
    SvgText();
    SvgText(const SvgText& other);
    SvgText(float _x, float _y, float _fontSize, const string& _content);
	SvgText(float _x, float _y, float _fontSize, const string& _content, const string& _textAnchor);
    ~SvgText();

    void accept(Renderer& renderer) override;

    // Setters
    void setFontSize(float size);
    void setTextAnchor(const string& anchor);

    // Getters 
    float getX() const;
    float getY() const;
    float getFontSize() const;
    const string& getContent() const;
    const string& getTextAnchor() const;
};
