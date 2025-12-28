#pragma once
#include "SvgElement.h"
#include "Gradient.h"
#include "framework.h"

using namespace std;
using namespace Gdiplus;

class SvgDocument {
private:
    vector<unique_ptr<SvgElement>> elements;

    // The Pool
    unordered_map<string, unique_ptr<Gradient>> gradientPool;

public:
    void addElement(unique_ptr<SvgElement> element);

    // Helper to add gradient
    void addGradient(unique_ptr<Gradient> grad) {
        if (!grad->id.empty()) {
            gradientPool[grad->id] = move(grad);
        }
    }

    // Helper to get gradient (used by Renderer)
    const Gradient* getGradient(const string& id) const {
        auto it = gradientPool.find(id);
        if (it != gradientPool.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    void draw(Graphics& graphic);
};