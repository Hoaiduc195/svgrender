#pragma once
#include "SvgElement.h"
#include "framework.h"

class Renderer;


class SvgEllipse : public SvgElement {
    private:
        float cx;
        float cy;
        float rx;
        float ry;
    public:
        SvgEllipse(float cx, float cy, float rx, float ry);
        void accept(Renderer& renderer) override;

        float getCx() const { return cx; }
        float getCy() const { return cy; }
        float getRx() const { return rx; }
        float getRy() const { return ry; }
};