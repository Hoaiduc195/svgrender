#pragma once
#include "SvgElement.h"
#include "framework.h"

class SvgDocument : public SvgElement {
private:
    vector<unique_ptr<SvgElement>> elements;
public:
    SvgDocument() = default;
    void addElement(unique_ptr<SvgElement>);
    void draw(Graphics& graphics) override;
};