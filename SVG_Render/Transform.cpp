#include "Transform.h"
#include <math.h>

Matrix3x3 Matrix3x3::Translate(float dx, float dy){
    return{{
        {1,0,dx},
        {0,1,dy},
        {0,0,1}
    }};;
}

Matrix3x3 Matrix3x3::Scale(float sx, float sy){
    return{{
        {sx,0,0},
        {0,sy,0},
        {0,0,1}
    }};;
}

Matrix3x3 Matrix3x3::Rotate(float angle){
    float theta = angle * (3.1415926535f  / 180.0f);
    return{{
        {cos(theta), -sin(theta), 0},
        {sin(theta), cos(theta), 0},
        {0, 0, 1}
    }};;
}

Matrix3x3 Matrix3x3::SkewX(float angle){
    float theta = angle * (3.1415926535f  / 180.0f);
    return{{
        {1, tan(theta), 0},
        {0, 1, 0},
        {0, 0, 1}
    }};;
}

Matrix3x3 Matrix3x3::SkewY(float angle){
    float theta = angle * (3.1415926535f  / 180.0f);
    return{{
        {1, 0, 0},
        {tan(theta), 1, 0},
        {0, 0, 1}
    }};;
}

Matrix3x3& Matrix3x3::operator*=(const Matrix3x3 &other){
    float m00 = matrix[0][0], m01 = matrix[0][1], m02 = matrix[0][2];
    float m10 = matrix[1][0], m11 = matrix[1][1], m12 = matrix[1][2];
    float m20 = matrix[2][0], m21 = matrix[2][1], m22 = matrix[2][2];

    matrix[0][0] = (m00 * other.matrix[0][0]) + (m01 * other.matrix[1][0]) + (m02 * other.matrix[2][0]);
    matrix[0][1] = (m00 * other.matrix[0][1]) + (m01 * other.matrix[1][1]) + (m02 * other.matrix[2][1]);
    matrix[0][2] = (m00 * other.matrix[0][2]) + (m01 * other.matrix[1][2]) + (m02 * other.matrix[2][2]);

    matrix[1][0] = (m10 * other.matrix[0][0]) + (m11 * other.matrix[1][0]) + (m12 * other.matrix[2][0]);
    matrix[1][1] = (m10 * other.matrix[0][1]) + (m11 * other.matrix[1][1]) + (m12 * other.matrix[2][1]);
    matrix[1][2] = (m10 * other.matrix[0][2]) + (m11 * other.matrix[1][2]) + (m12 * other.matrix[2][2]);

    matrix[2][0] = (m20 * other.matrix[0][0]) + (m21 * other.matrix[1][0]) + (m22 * other.matrix[2][0]);
    matrix[2][1] = (m20 * other.matrix[0][1]) + (m21 * other.matrix[1][1]) + (m22 * other.matrix[2][1]);
    matrix[2][2] = (m20 * other.matrix[0][2]) + (m21 * other.matrix[1][2]) + (m22 * other.matrix[2][2]);
    
    return *this;
}

Transform::Transform(){
    this->m = Matrix3x3::Identity();
}

void Transform::translate(float dx, float dy){
    m *= Matrix3x3::Translate(dx, dy);
}

void Transform::scale(float sx, float sy){
    m *= Matrix3x3::Scale(sx, sy);
}

void Transform::rotate(float angle){
    m *= Matrix3x3::Rotate(angle);
}

void Transform::skewX(float angle){
    m *= Matrix3x3::SkewX(angle);
}

void Transform::skewY(float angle){
    m *= Matrix3x3::SkewY(angle);
}

void Transform::matrix(float a, float b, float c, float d, float e, float f){
    // Apply 2D affine transformation matrix multiplication
    // new matrix = this * [[a, c, e], [b, d, f], [0, 0, 1]]
    float m00 = m.matrix[0][0], m01 = m.matrix[0][1], m02 = m.matrix[0][2];
    float m10 = m.matrix[1][0], m11 = m.matrix[1][1], m12 = m.matrix[1][2];

    m.matrix[0][0] = (m00 * a) + (m01 * b);
    m.matrix[0][1] = (m00 * c) + (m01 * d);
    m.matrix[0][2] = (m00 * e) + (m01 * f) + m02;

    m.matrix[1][0] = (m10 * a) + (m11 * b);
    m.matrix[1][1] = (m10 * c) + (m11 * d);
    m.matrix[1][2] = (m10 * e) + (m11 * f) + m12;
}

Matrix3x3 Transform::getMatrix() const {
    return this->m;
}