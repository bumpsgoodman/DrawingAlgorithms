#pragma once

class Canvas final
{
public:
    Canvas() = default;
    Canvas(const Canvas&) = delete;
    Canvas(Canvas&&) = delete;
    Canvas& operator=(const Canvas&) = delete;
    Canvas& operator=(Canvas&&) = delete;
    ~Canvas();

    bool Initialize(DDraw* ddraw, const uint32_t cellSize);
    void Cleanup();
    void Clear();

    void Update(const float deltaTime);
    void Draw() const;

    inline uint32_t GetCellSize() const { return mCellSize; }
    inline uint32_t GetRows() const { return mRows; }
    inline uint32_t GetCols() const { return mCols; }
    inline Color GetPixel(const Vector2& pos) const;

    void DrawPixel(const Vector2& pos, const Color& color, const bool bCommand=true);
    void RemovePixel(const Vector2& pos, const bool bCommand=true);
    void DrawLineDDA(const Vector2& startPos, const Vector2& endPos, const Color& color);
    void DrawLineBresenham(const Vector2& startPos, const Vector2& endPos, const Color& color);
    void DrawCircle(const Vector2& centerPos, const float radius, const Color& color);
    void DrawTriangle(const Vector2& pos0, const Vector2& pos1, const Vector2& pos2, const Color& color);
    void DrawRectangle(const Vector2& pos0, const Vector2& pos1, const Color& color);

    inline bool CanUndo() const { return !mCommandHistory.empty(); }
    inline bool CanRedo() const { return !mRedoCommandHistory.empty(); }
    bool Undo();
    bool Redo();

    BOOL CALLBACK DialogEventHandler(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);

    inline static void ToCanvasCoord(const Vector2& screenPos, const uint32_t cellSize, Vector2* outCanvasPos);

private:
    inline void clearCanvas(Color* canvas);
    inline bool isValidPos(const Vector2& pos) const;
    inline bool recordPosToHistory(const Vector2& pos, std::vector<Vector2>* outPosVect);
    void windowToViewport(const Vector2& posInWindow, Vector2* outPosInViewport);

private:
    DDraw* mDDraw = nullptr;
    HWND mhDialog = nullptr;

    uint32_t mCellSize = 0;
    uint32_t mRows = 0;
    uint32_t mCols = 0;

    Color* mCanvas = nullptr;
    Color* mFrontCanvas = nullptr;
    Color* mBackCanvas = nullptr;
    std::vector<Color>* mPixelHistory = nullptr;
    std::vector<std::vector<Vector2>> mCommandHistory;
    std::vector<std::vector<Vector2>> mRedoCommandHistory;
    std::vector<std::vector<Color>> mRedoPixelHistory;
    std::vector<std::vector<Vector2>> mPointsHistory;
    std::vector<std::vector<Vector2>> mRedoPointsHistory;

    eCommandType mCurCommandType = eCommandType::Pixel;
    eCommandType mLastCommandType = eCommandType::Pixel;

    float mRadius = 0;

    bool mbClipped = false;
    bool mbClippingRegion = false;
    Vector2 mClippingRegionLeftTopPos = { -1, -1 };
    Vector2 mClippingRegionRightBottomPos = { -1, -1 };
};

inline Color Canvas::GetPixel(const Vector2& pos) const
{
    AssertW(isValidPos(pos), L"Invalid pos");
    return mCanvas[ROUND(pos.Y) * mCols + ROUND(pos.X)];
}

inline void Canvas::clearCanvas(Color* canvas)
{
    for (uint32_t y = 0; y < mRows; ++y)
    {
        for (uint32_t x = 0; x < mCols; ++x)
        {
            canvas[y * mCols + x] = colors::WHITE;
        }
    }
}

inline bool Canvas::isValidPos(const Vector2& pos) const
{
    return ((uint32_t)ROUND(pos.X) < (uint32_t)mCols && (uint32_t)ROUND(pos.Y) < (uint32_t)mRows);
}

inline void Canvas::ToCanvasCoord(const Vector2& screenPos, const uint32_t cellSize, Vector2* outCanvasPos)
{
    AssertW(outCanvasPos != nullptr, L"outCanvasCoord is nullptr");

    outCanvasPos->X = (float)(ROUND(screenPos.X) / (int32_t)cellSize);
    outCanvasPos->Y = (float)(ROUND(screenPos.Y) / (int32_t)cellSize);
}

inline bool Canvas::recordPosToHistory(const Vector2& pos, std::vector<Vector2>* outPosVect)
{
    AssertW(outPosVect != nullptr, L"outPosVect is nullptr");

    if (!isValidPos(pos) || mCanvas == mBackCanvas)
    {
        return false;
    }

    outPosVect->push_back(pos);
    mPixelHistory[ROUND(pos.Y) * mCols + ROUND(pos.X)].push_back(GetPixel(pos));

    return true;
}