#pragma once
#include "framework.h"


template <typename T>
T clamp(T value, T minVal, T maxVal) {
    return max(minVal, min(value, maxVal));
}

class Renderer; // forward

class SvgElement {
    protected:
        Color fill;
        Color stroke;
        float strokeWidth;
        float strokeOpacity;
        float fillOpacity;

    public:
        virtual void accept(Renderer& renderer) = 0;
        virtual ~SvgElement() = default;
        SvgElement();

        // Setters
        void setFill(const Color& fillColor);
        void setStroke(const Color& strokeColor);
        void setStrokeWidth(float width);
        void setStrokeOpacity(float opacity);
        void setFillOpacity(float opacity);

        // Getters for renderer
        const Color& getFill() const { return fill; }
        const Color& getStroke() const { return stroke; }
        float getStrokeWidth() const { return strokeWidth; }
        float getStrokeOpacity() const { return strokeOpacity; }
        float getFillOpacity() const { return fillOpacity; }
};