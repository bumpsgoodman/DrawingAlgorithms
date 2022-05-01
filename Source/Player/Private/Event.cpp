#include "Precompiled.h"

namespace event
{
    namespace mouse
    {
        static bool sbClicked = false;
        static uint32_t sClickedX;
        static uint32_t sClickedY;

        void OnClick(const uint32_t x, const uint32_t y)
        {
            sbClicked = true;
            sClickedX = x;
            sClickedY = y;
        }

        bool IsClicked()
        {
            return sbClicked;
        }

        void Release()
        {
            sbClicked = false;
            sClickedX = -1;
            sClickedY = -1;
        }

        uint32_t GetX()
        {
            return sClickedX;
        }

        uint32_t GetY()
        {
            return sClickedY;
        }
    }

    namespace keyboard
    {
        static bool sbKeyMap[256] = {};

        void KeyDown(const uint8_t keyCode)
        {
            sbKeyMap[keyCode] = true;
        }

        void KeyUp(const uint8_t keyCode)
        {
            sbKeyMap[keyCode] = false;
        }

        void Release()
        {
            memset(sbKeyMap, false, sizeof(sbKeyMap));
        }

        bool IsKeyPressed(const uint8_t keyCode)
        {
            return sbKeyMap[keyCode];
        }
    }
}