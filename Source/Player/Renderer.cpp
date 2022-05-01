#include "Precompiled.h"

Renderer::~Renderer()
{
    Shutdown();
}

void Renderer::Shutdown()
{
    SAFE_DELETE(mCanvas);
    SAFE_DELETE(mDDraw);
}

bool Renderer::Initialize(HWND hWnd)
{
    mhWnd = hWnd;

    mDDraw = new DDraw;
    if (!mDDraw->Initialize(hWnd))
    {
        AssertW(false, L"Failed to Create ddraw object");
        return false;
    }

    mCanvas = new Canvas;
    if (!mCanvas->Initialize(mDDraw, 5))
    {
        AssertW(false, L"Failed to Create canvas");
        return false;
    }

    QueryPerformanceFrequency(&mFrequency);
    QueryPerformanceCounter(&mPrevCounter);

    return true;
}

void Renderer::Tick()
{
    LARGE_INTEGER curCounter;

    QueryPerformanceCounter(&curCounter);
    const double dElapsedTick = (((double)curCounter.QuadPart - (double)mPrevCounter.QuadPart) / (double)mFrequency.QuadPart * 1000.0);
    const float fElapsedTick = (float)dElapsedTick;

    const float deltaTime = fElapsedTick / TICKS_PER_FRAME;
    if (fElapsedTick > TICKS_PER_FRAME)
    {
        update(deltaTime);
        draw();

        mPrevCounter = curCounter;
    }
}

void Renderer::update(const float deltaTime)
{
    mCanvas->Update(deltaTime);
}

void Renderer::draw() const
{
    mDDraw->BeginDraw();
    {
        mCanvas->Draw();
    }
    mDDraw->EndDraw();

    mDDraw->Blt();
}