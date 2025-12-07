#pragma once
#include "framework.h"
#include "Transform.h"

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
		Transform transform;

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
        void setTransform(const Transform& t);

        // Getters for renderer
        const Color& getFill() const;
        const Color& getStroke() const;
        float getStrokeWidth() const;
        float getStrokeOpacity() const;
        float getFillOpacity() const;
        Transform getTransform() const;
};