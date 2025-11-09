#include"SvgText.h"
#include <vector>
#include <string>

SvgText::~SvgText() {}

SvgText::SvgText() {
	x = 0.0f;
	y = 0.0f;
	content = "";
	fontSize = 12.0f;
}

SvgText::SvgText(const SvgText& other) {
	*this = other;
}

SvgText::SvgText(float _x, float _y, float _fontSize, const string& _content) {
	x = _x;
	y = _y;
	fontSize = _fontSize;
	content = _content;
}


void SvgText::draw(Graphics& g) {
    g.SetTextContrast(0);
    g.SetSmoothingMode(SmoothingModeHighQuality);
    g.SetCompositingQuality(CompositingQualityHighQuality);
    g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
    g.SetPixelOffsetMode(PixelOffsetModeHighQuality);
    g.SetPageUnit(UnitPixel);

    StringFormat format; 

    FontFamily fontFamily(L"Times New Roman");
    Font font(&fontFamily, fontSize, FontStyleRegular, UnitPixel);

    SolidBrush brush(Color(
        static_cast<BYTE>(fillOpacity * 255),
        fill.GetR(),
        fill.GetG(),
        fill.GetB()
    ));

    RectF layoutRect(x, y - font.GetHeight(&g), 1000, font.GetHeight(&g));
    g.DrawString(
        std::wstring(content.begin(), content.end()).c_str(),
        -1,
        &font,
        layoutRect,
        &format,
        &brush
    );
}


void SvgText::setFontSize(float size) {
    fontSize = size;
}
