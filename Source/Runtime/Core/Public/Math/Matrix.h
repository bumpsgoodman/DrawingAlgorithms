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
};