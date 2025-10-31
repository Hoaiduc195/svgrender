#include<iostream>
#include <gdiplus.h>

using namespace std;
using namespace Gdiplus;

class SvgElement {
    private:
        string id;
        Color fill;
        Color stroke;
        float strokeWidth;
        float strokeOpacity;
        float fillOpacity;
    public:
        virtual void draw(Graphics& graphics) = 0; 
        virtual ~SvgElement() = default;
        SvgElement() {
            cout << "SvgElement created." << endl;
        }
        
        void render() {
            cout << "Rendering SVG Element." << endl;
        }


};