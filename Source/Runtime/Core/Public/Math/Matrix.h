#pragma once

class Matrix
{
public:
    Matrix() = delete;
    Matrix(const Matrix&) = delete;
    Matrix(const Matrix&&) = delete;
    Matrix& operator=(const Matrix&) = delete;
    ~Matrix() = default;

    static void MulMatrix13x33(const float matrix13[], const float matrix33[][3], float outMatrix13[]);
    static void MulMatrix33x33(const float matrix33_0[][3], const float matrix33_1[][3], float outMatrix33[][3]);
};