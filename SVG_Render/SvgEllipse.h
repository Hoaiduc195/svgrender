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
        void accept(Renderer& renderer) const override;

		// Getters
        float getCx() const;
        float getCy() const;
        float getRx() const;
        float getRy() const;
        RectF getBoundingBox() const override;
};