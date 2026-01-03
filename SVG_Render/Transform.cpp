#include "Transform.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif



Matrix3x3 Matrix3x3::Identity()
{
    return { {{1, 0, 0},
             {0, 1, 0},
             {0, 0, 1}} };
}

Matrix3x3 Matrix3x3::Translate(double dx, double dy)
{
    return { {{1, 0, dx},
             {0, 1, dy},
             {0, 0, 1}} };
}

Matrix3x3 Matrix3x3::Scale(double sx, double sy)
{
    return { {{sx, 0, 0},
             {0, sy, 0},
             {0, 0, 1}} };
}


Matrix3x3 Matrix3x3::Rotate(double angle)
{
    double theta = angle * (M_PI / 180.0);
    double c = cos(theta);
    double s = sin(theta);
    return { {{c, -s, 0},
             {s, c, 0},
             {0, 0, 1}} };
}


Matrix3x3 Matrix3x3::SkewX(double angle)
{
    double theta = angle * (M_PI / 180.0);
    return { {{1, tan(theta), 0},
             {0, 1, 0},
             {0, 0, 1}} };
}

Matrix3x3 Matrix3x3::SkewY(double angle)
{
    double theta = angle * (M_PI / 180.0);
    return { {{1, 0, 0},
             {tan(theta), 1, 0},
             {0, 0, 1}} };
}


Matrix3x3& Matrix3x3::operator*=(const Matrix3x3& other)
{
    Matrix3x3 result;
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++)
            result.matrix[r][c] =
            matrix[r][0] * other.matrix[0][c] +
            matrix[r][1] * other.matrix[1][c] +
            matrix[r][2] * other.matrix[2][c];

    *this = result;
    return *this;
}


Transform::Transform()
{
    this->m = Matrix3x3::Identity();
}

void Transform::translate(double dx, double dy)
{
    m *= Matrix3x3::Translate(dx, dy);
}

void Transform::scale(double sx, double sy)
{
    m *= Matrix3x3::Scale(sx, sy);
}

void Transform::rotate(double angle)
{
    m *= Matrix3x3::Rotate(angle);
}

void Transform::skewX(double angle)
{
    m *= Matrix3x3::SkewX(angle);
}

void Transform::skewY(double angle)
{
    m *= Matrix3x3::SkewY(angle);
}

void Transform::matrix(double a, double b, double c, double d, double e, double f)
{
    Matrix3x3 t = { {
        {a, c, e},
        {b, d, f},
        {0, 0, 1}
    } };

    m *= t;
}

Matrix3x3 Transform::getMatrix() const
{
    return this->m;
}