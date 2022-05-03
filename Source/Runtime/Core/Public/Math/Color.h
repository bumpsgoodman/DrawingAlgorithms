#pragma once

// RGBA
struct Color
{
    Color() : Color(0.0f, 0.0f, 0.0f, 0.0f) {}

    Color(const float rgba[4])
        : Color(rgba[0], rgba[1], rgba[2], rgba[3])
    {
    }

    Color(const float r, const float g, const float b, const float a)
        : R(r)
        , G(g)
        , B(b)
        , A(a)
    {
    }

    Color(const uint32_t r, const uint32_t g, const uint32_t b, const uint32_t a)
        : Color((float)r, (float)g, (float)b, (float)a)
    {
    }

    union
    {
        float RGBA[4];

        struct
        {
            float R;
            float G;
            float B;
            float A;
        };
    };

    static inline uint32_t ToHex(const Color color)
    {
        uint32_t val =
            ((uint32_t)(color.R * 255.0f) << 24
                | (uint32_t)(color.G * 255.0f) << 16
                | (uint32_t)(color.B * 255.0f) << 8
                | (uint32_t)(color.A * 255.0f));
        return val;
    }

    static inline uint32_t ToARGBHex(const Color color)
    {
        uint32_t val =
            ((uint32_t)(color.A * 255.0f) << 24
                | (uint32_t)(color.R * 255.0f) << 16
                | (uint32_t)(color.G * 255.0f) << 8
                | (uint32_t)(color.B * 255.0f));
        return val;
    }
};

// RGBA
namespace colors
{
    const Color WHITE = { 1.000000f, 1.0000000f, 1.0000000f, 1.000000f };
    const Color BLACK = { 0.000000f, 0.0000000f, 0.0000000f, 1.000000f };
    const Color RED = { 1.000000f, 0.0000000f, 0.0000000f, 1.000000f };
    const Color BLUE = { 0.000000f, 0.0000000f, 1.0000000f, 1.000000f };
    const Color GREEN = { 0.000000f, 1.0000000f, 0.0000000f, 1.000000f };
}