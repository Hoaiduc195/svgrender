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

    matrix[0][0] = (matrix[0][0] * other.matrix[0][0]) + (matrix[0][1] * other.matrix[1][0]);

    matrix[1][0] = (matrix[1][0] * other.matrix[0][0]) + (matrix[1][1] * other.matrix[1][0]);

    matrix[0][1] = (matrix[0][0] * other.matrix[0][1]) + (matrix[0][1] * other.matrix[1][1]);

    matrix[1][1] = (matrix[1][0] * other.matrix[0][1]) + (matrix[1][1] * other.matrix[1][1]);

    matrix[0][2] = (matrix[0][0] * other.matrix[0][2]) + (matrix[0][1] * other.matrix[1][2]) + matrix[0][2];

    matrix[1][2] = (matrix[1][0] * other.matrix[0][2]) + (matrix[1][1] * other.matrix[1][2]) + matrix[1][2];
    
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

    m.matrix[0][0] = (m.matrix[0][0] * a) + (m.matrix[0][1] * b);

    m.matrix[1][0] = (m.matrix[1][0] * a) + (m.matrix[1][1] * b);

    m.matrix[0][1] = (m.matrix[0][0] * c) + (m.matrix[0][1] * d);

    m.matrix[1][1] = (m.matrix[1][0] * c) + (m.matrix[1][1] * d);

    m.matrix[0][2] = (m.matrix[0][0] * e) + (m.matrix[0][1] * f) + m.matrix[0][2];

    m.matrix[1][2] = (m.matrix[1][0] * e) + (m.matrix[1][1] * f) + m.matrix[1][2];

}

Matrix3x3 Transform::getMatrix(){
    return this->m;
}