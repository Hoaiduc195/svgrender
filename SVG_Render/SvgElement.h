#pragma once
#include "Color.h"
#include "Transform.h"
#include "framework.h"

// Enum to determine what kind of fill this element uses
enum class FillType {
    None,
    SolidColor,
    Gradient
};

class Renderer; // Forward declaration

class SvgElement {
protected:
    // Fill Properties
    FillType fillType;
    Color fill;             // Used if fillType == SolidColor
    std::string gradientId; // Used if fillType == Gradient
    float fillOpacity;

    // Stroke Properties
    Color stroke;
    float strokeWidth;
    float strokeOpacity;

    Transform transform;

public:
    SvgElement();
    virtual ~SvgElement() = default;

    // Setters
    void setFill(const Color& color);
    void setFillGradient(const std::string& id); // New method
    void setFillOpacity(float opacity);
    void setStroke(const Color& color);
    void setStrokeWidth(float width);
    void setStrokeOpacity(float opacity);
    void setTransform(const Transform& transform);

    // Getters
    FillType getFillType() const { return fillType; }
    const Color& getFill() const;
    const std::string& getGradientId() const; // New getter
    float getFillOpacity() const;
    const Color& getStroke() const;
    float getStrokeWidth() const;
    float getStrokeOpacity() const;
    Transform getTransform() const;

    virtual void accept(Renderer& renderer) const = 0;
    virtual RectF getBoundingBox() const = 0;
};