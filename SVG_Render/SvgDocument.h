#pragma once
#include "SvgElement.h"
#include "framework.h"

class SvgDocument : public SvgElement {
private:
    vector<unique_ptr<SvgElement>> children;
public:
    SvgDocument() = default;
    void addChild(unique_ptr<SvgElement>);
    void draw(Graphics& graphics) override;
};