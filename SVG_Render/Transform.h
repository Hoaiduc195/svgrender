#pragma once
#include<vector>
#include<iostream>
#include<math.h>
class Matrix3x3{
    public:
        float matrix[3][3];
        static Matrix3x3 Identity() {
            return { {
                {1,0,0},
                {0,1,0},
                {0,0,1}
            }};
        }
        static Matrix3x3 Translate(float dx, float dy);
        static Matrix3x3 Scale(float sx, float sy);
        static Matrix3x3 Rotate(float angle);
        static Matrix3x3 SkewX(float angle);
        static Matrix3x3 SkewY(float angle);
        Matrix3x3& operator*=(const Matrix3x3 &other);
};
class Transform{
    private:
        Matrix3x3 m;
    public:
        Transform();
        void translate(float dx, float dy);
        void rotate(float angle);
        void scale(float sx, float sy);
        void skewX(float angle);
        void skewY(float angle);
        void matrix(float a, float b, float c, float d, float e, float f);
        Matrix3x3 getMatrix();
};