#pragma once

namespace event
{
    namespace mouse
    {
        void OnClick(const uint32_t x, const uint32_t y);
        bool IsClicked();
        void Release();
        uint32_t GetX();
        uint32_t GetY();
    }

    namespace keyboard
    {
        void KeyDown(const uint8_t keyCode);
        void KeyUp(const uint8_t keyCode);
        void Release();
        bool IsKeyPressed(const uint8_t keyCode);
    }
}