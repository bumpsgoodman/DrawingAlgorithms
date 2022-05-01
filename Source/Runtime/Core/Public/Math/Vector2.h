#pragma once

struct Vector2
{
    union
    {
        struct
        {
            float X;
            float Y;
        };

        float XY[2];
    };

    Vector2() : Vector2(0.0f, 0.0f) {}
    Vector2(const int32_t x, const int32_t y) : Vector2((float)x, (float)y) {}
    Vector2(const float x, const float y) : X(x), Y(y) {}

    inline Vector2 operator+(const Vector2& v);
    inline Vector2 operator-(const Vector2& v);
    inline Vector2 operator*(const Vector2& v);

    inline Vector2 operator+(const float scalar);
    inline Vector2 operator-(const float scalar);
    inline Vector2 operator*(const float scalar);
    inline Vector2 operator/(const float scalar);

    inline Vector2& operator+=(const Vector2& v);
    inline Vector2& operator-=(const Vector2& v);
    inline Vector2& operator*=(const float scalar);
    inline Vector2& operator/=(const float scalar);

    static inline Vector2 Zero();
    static inline Vector2 One();
};

inline Vector2 Vector2::operator+(const Vector2& v)
{
    return Vector2(X + v.X, Y + v.Y);
}

inline Vector2 Vector2::operator-(const Vector2& v)
{
    return Vector2(X - v.X, Y - v.Y);
}

inline Vector2 Vector2::operator*(const Vector2& v)
{
    return Vector2(X * v.X, Y * v.Y);
}

inline Vector2 Vector2::operator+(const float scalar)
{
    return Vector2(X + scalar, Y + scalar);
}

inline Vector2 Vector2::operator-(const float scalar)
{
    return Vector2(X - scalar, Y - scalar);
}

inline Vector2 Vector2::operator*(const float scalar)
{
    return Vector2(X * scalar, Y * scalar);
}

inline Vector2 Vector2::operator/(const float scalar)
{
    return Vector2(X / scalar, Y / scalar);
}

inline Vector2& Vector2::operator+=(const Vector2& v)
{
    X += v.X;
    Y += v.Y;
    return *this;
}

inline Vector2& Vector2::operator-=(const Vector2& v)
{
    X -= v.X;
    Y -= v.Y;
    return *this;
}

inline Vector2& Vector2::operator*=(const float scalar)
{
    X *= scalar;
    Y *= scalar;
    return *this;
}

inline Vector2& Vector2::operator/=(const float scalar)
{
    X /= scalar;
    Y /= scalar;
    return *this;
}

inline Vector2 Vector2::Zero()
{
    return Vector2(0, 0);
}

inline Vector2 Vector2::One()
{
    return Vector2(1, 1);
}