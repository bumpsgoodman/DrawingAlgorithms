#pragma once

class Renderer final
{
public:
    Renderer() = default;
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&) = delete;
    ~Renderer();

    bool Initialize(HWND hWnd);
    void Shutdown();

    void Tick();

    inline Canvas* GetCanvas() { return mCanvas; }

    inline void UpdateWindowPos() { mDDraw->UpdateWindowPos(); }

private:
    void update(const float deltaTime);
    void draw() const;

private:
    HWND mhWnd = nullptr;
    DDraw* mDDraw = nullptr;
    Canvas* mCanvas = nullptr;

private:
    static constexpr uint32_t FPS = 60;
    static constexpr float TICKS_PER_FRAME = 1000.0f / (float)FPS;

    LARGE_INTEGER mFrequency = {};
    LARGE_INTEGER mPrevCounter = {};
};