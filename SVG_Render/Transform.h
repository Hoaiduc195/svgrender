#pragma once
#include <vector>
#include <iostream>
#include <math.h>

class Matrix3x3
{
public:
    double matrix[3][3];
    static Matrix3x3 Identity();
    static Matrix3x3 Translate(double dx, double dy);
    static Matrix3x3 Scale(double sx, double sy);
    static Matrix3x3 Rotate(double angle);
    static Matrix3x3 SkewX(double angle);
    static Matrix3x3 SkewY(double angle);
    Matrix3x3 &operator*=(const Matrix3x3 &other);
};

class Transform
{
private:
    Matrix3x3 m;
public:
    Transform();
    void translate(double dx, double dy);
    void rotate(double angle);
    void scale(double sx, double sy);
    void skewX(double angle);
    void skewY(double angle);
    void matrix(double a, double b, double c, double d, double e, double f);
    Matrix3x3 getMatrix() const;
};