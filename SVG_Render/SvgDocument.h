#include "SvgElement.h"
#include <vector>

class SvgDocument : public SvgElement {
private:
    vector<unique_ptr<SvgElement>> children;
public:
    SvgDocument() = default;
    void addChild(unique_ptr<SvgElement>);
    void draw(Graphics&) override;
};