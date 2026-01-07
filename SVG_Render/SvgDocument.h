#pragma once
#include "SvgElement.h"
#include "Gradient.h"
#include "framework.h"

using namespace std;
using namespace Gdiplus;

class SvgDocument {
private:
    vector<unique_ptr<SvgElement>> elements;
    unordered_map<string, unique_ptr<Gradient>> gradientPool;

public:
    void addElement(unique_ptr<SvgElement> element);
    void addGradient(unique_ptr<Gradient> grad);
    const Gradient* getGradient(const string& id) const;
    void draw(Graphics& graphic);
    RectF getBoundingBox() const;
};