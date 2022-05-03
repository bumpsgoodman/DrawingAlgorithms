#include "Precompiled.h"
#include "RegionCode.h"

static bool ClipCohem(Vector2* inOutStartPos, Vector2* inOutEndPos, const Vector2& regionLeftTopPos, const Vector2& regionRightBottomPos);
static bool ClipLiang(Vector2* inOutStartPos, Vector2* inOutEndPos, const Vector2& regionLeftTopPos, const Vector2& regionRightBottomPos);

Canvas::~Canvas()
{
    Cleanup();
}

bool Canvas::Initialize(DDraw* ddraw, const uint32_t cellSize)
{
    if (ddraw == nullptr)
    {
        AssertW(ddraw != nullptr, L"ddraw is nullptr");
        return false;
    }

    mDDraw = ddraw;
    mCellSize = cellSize;

    mRows = ddraw->GetHeight() / cellSize;
    mCols = ddraw->GetWidth() / cellSize;

    mFrontCanvas = new Color[mRows * mCols];
    mBackCanvas = new Color[mRows * mCols];
    mPixelHistory = new std::vector<Color>[mRows * mCols];

    mCanvas = mFrontCanvas;
    Clear();

    return true;
}

void Canvas::Cleanup()
{
    SAFE_DELETE_ARRAY(mPixelHistory);
    SAFE_DELETE_ARRAY(mFrontCanvas);
    SAFE_DELETE_ARRAY(mBackCanvas);
}

void Canvas::Clear()
{
    mCanvas = mFrontCanvas;

    for (uint32_t y = 0; y < mRows; ++y)
    {
        for (uint32_t x = 0; x < mCols; ++x)
        {
            mFrontCanvas[y * mCols + x] = colors::WHITE;
            mBackCanvas[y * mCols + x] = colors::WHITE;
            mPixelHistory[y * mCols + x].clear();
        }
    }

    mCommandHistory.clear();
    mRedoCommandHistory.clear();
    mRedoPixelHistory.clear();
    mPointsHistory.clear();
    mRedoPointsHistory.clear();

    mbClipped = false;
    mbClippingRegion = false;

    if (mhDialog != nullptr)
    {
        Button_SetCheck(GetDlgItem(mhDialog, IDC_CHECK_CLIP), BST_UNCHECKED);
    }
}

void Canvas::Update(const float deltaTime)
{
    static Vector2 p0(-1.0f, -1.0f);
    static Vector2 p1(-1.0f, -1.0f);
    static Vector2 p2(-1.0f, -1.0f);

    if (event::mouse::IsClicked())
    {
        if (mbClipped)
        {
            event::mouse::Release();
            return;
        }

        Vector2 mousePos((float)event::mouse::GetX(), (float)event::mouse::GetY());

        switch (mCurCommandType)
        {
        case eCommandType::Pixel:
            Canvas::ToCanvasCoord(mousePos, GetCellSize(), &p0);
            DrawPixel(p0, colors::RED);

            p0.X = -1;
            break;
        case eCommandType::Remove:
            Canvas::ToCanvasCoord(mousePos, GetCellSize(), &p0);
            RemovePixel(p0);

            p0.X = -1;
            break;
        case eCommandType::LineDDA:
            // fall-through
        case eCommandType::LineBresenham:
            if (p0.X == -1)
            {
                Canvas::ToCanvasCoord(mousePos, GetCellSize(), &p0);
            }
            else
            {
                Canvas::ToCanvasCoord(mousePos, GetCellSize(), &p1);
                DrawLineBresenham(p0, p1, colors::RED);

                p0.X = -1;
                p1.X = -1;
            }
            break;
        case eCommandType::Circle:
        {
            wchar_t radiusText[128];
            GetDlgItemText(mhDialog, IDC_EDIT_RADIUS, radiusText, 128);
            float radius = (float)_wtof(radiusText);

            Canvas::ToCanvasCoord(mousePos, GetCellSize(), &p0);
            DrawCircle(p0, radius, colors::RED);

            p0.X = -1;
            break;
        }
        case eCommandType::Triangle:
            if (p0.X == -1)
            {
                Canvas::ToCanvasCoord(mousePos, GetCellSize(), &p0);
            }
            else if (p1.X == -1)
            {
                Canvas::ToCanvasCoord(mousePos, GetCellSize(), &p1);
            }
            else
            {
                Canvas::ToCanvasCoord(mousePos, GetCellSize(), &p2);
                DrawTriangle(p0, p1, p2, colors::RED);

                p0.X = -1;
                p1.X = -1;
                p2.X = -1;
            }
            break;
        case eCommandType::Region:
            if (mbClippingRegion)
            {
                break;
            }

            if (p0.X == -1)
            {
                Canvas::ToCanvasCoord(mousePos, GetCellSize(), &p0);
            }
            else
            {
                Canvas::ToCanvasCoord(mousePos, GetCellSize(), &p1);

                mClippingRegionLeftTopPos = p0;
                mClippingRegionRightBottomPos = p1;
                mbClippingRegion = true;

                p0.X = -1;
                p1.X = -1;
            }
            break;
        default:
            return;
        }

        event::mouse::Release();
    }

    if (IsDlgButtonChecked(mhDialog, IDC_CHECK_CLIP) && mbClippingRegion)
    {
        if (mbClipped)
        {
            return;
        }

        mCanvas = mBackCanvas;
        clearCanvas(mCanvas);

        for (uint32_t i = 0; i < mPointsHistory.size(); ++i)
        {
            std::vector<Vector2> pointsVect = mPointsHistory[i];
            Vector2 startPos;
            Vector2 endPos;
            switch (pointsVect.size())
            {
            case 1:
            {
                startPos = pointsVect[0];
                endPos = pointsVect[0];
                if (ClipCohem(&startPos, &endPos, mClippingRegionLeftTopPos, mClippingRegionRightBottomPos))
                {
                    Vector2 startViewportPos;
                    windowToViewport(startPos, &startViewportPos);

                    DrawPixel(startViewportPos, colors::RED);
                }
                break;
            }
            case 2:
            {
                startPos = pointsVect[0];
                endPos = pointsVect[1];
                if (ClipCohem(&startPos, &endPos, mClippingRegionLeftTopPos, mClippingRegionRightBottomPos))
                {
                    Vector2 startViewportPos;
                    Vector2 endViewportPos;
                    windowToViewport(startPos, &startViewportPos);
                    windowToViewport(endPos, &endViewportPos);

                    DrawLineBresenham(startViewportPos, endViewportPos, colors::RED);
                }
                break;
            }
            case 3:
            {
                startPos = pointsVect[0];
                endPos = pointsVect[1];
                Vector2 startViewportPos;
                Vector2 endViewportPos;
                if (ClipCohem(&startPos, &endPos, mClippingRegionLeftTopPos, mClippingRegionRightBottomPos))
                {
                    windowToViewport(startPos, &startViewportPos);
                    windowToViewport(endPos, &endViewportPos);

                    DrawLineBresenham(startViewportPos, endViewportPos, colors::RED);
                }

                startPos = pointsVect[1];
                endPos = pointsVect[2];
                if (ClipCohem(&startPos, &endPos, mClippingRegionLeftTopPos, mClippingRegionRightBottomPos))
                {
                    windowToViewport(startPos, &startViewportPos);
                    windowToViewport(endPos, &endViewportPos);

                    DrawLineBresenham(startViewportPos, endViewportPos, colors::RED);
                }

                startPos = pointsVect[2];
                endPos = pointsVect[0];
                if (ClipCohem(&startPos, &endPos, mClippingRegionLeftTopPos, mClippingRegionRightBottomPos))
                {
                    windowToViewport(startPos, &startViewportPos);
                    windowToViewport(endPos, &endViewportPos);

                    DrawLineBresenham(startViewportPos, endViewportPos, colors::RED);
                }
                break;
            }
            default:
                continue;
            }
        }

        mbClipped = true;
    }
    else
    {
        if (mbClipped)
        {
            mCanvas = mFrontCanvas;
            mbClipped = false;

            if (mhDialog != nullptr)
            {
                Button_SetCheck(GetDlgItem(mhDialog, IDC_CHECK_CLIP), BST_UNCHECKED);
            }
        }
    }
}

void Canvas::Draw() const
{
    const uint32_t WINDOW_WIDTH = mDDraw->GetWidth();
    const uint32_t WINDOW_HEIGHT = mDDraw->GetHeight();

    const uint32_t BOUNDARY = 1;

    for (uint32_t y = 0; y < mRows; ++y)
    {
        mDDraw->DrawLineDDA(0, y * mCellSize, WINDOW_WIDTH - 1, y * mCellSize, Color::ToARGBHex(colors::BLACK));
    }

    for (uint32_t x = 0; x < mCols; ++x)
    {
        mDDraw->DrawLineDDA(x * mCellSize, 0, x * mCellSize, WINDOW_HEIGHT - 1, Color::ToARGBHex(colors::BLACK));
    }

    for (uint32_t y = 0; y < mRows; ++y)
    {
        for (uint32_t x = 0; x < mCols; ++x)
        {
            mDDraw->DrawRectangle(x * mCellSize + BOUNDARY, y * mCellSize + BOUNDARY,
                mCellSize - BOUNDARY, mCellSize - BOUNDARY,
                Color::ToARGBHex(mCanvas[y * mCols + x]));
        }
    }

    if (mCanvas == mFrontCanvas && mbClippingRegion)
    {
        const Vector2& leftTop = mClippingRegionLeftTopPos;
        const Vector2& rightBottom = mClippingRegionRightBottomPos;
        const uint32_t width = ROUND(rightBottom.X - leftTop.X);
        const uint32_t height = ROUND(rightBottom.Y - leftTop.Y);

        for (uint32_t x = 0; x <= width; ++x)
        {
            mDDraw->DrawRectangle((x + ROUND(leftTop.X)) * mCellSize + BOUNDARY, ROUND(leftTop.Y) * mCellSize + BOUNDARY,
                mCellSize - BOUNDARY, mCellSize - BOUNDARY,
                Color::ToARGBHex(colors::BLUE));
            mDDraw->DrawRectangle((x + ROUND(leftTop.X)) * mCellSize + BOUNDARY, ROUND(rightBottom.Y) * mCellSize + BOUNDARY,
                mCellSize - BOUNDARY, mCellSize - BOUNDARY,
                Color::ToARGBHex(colors::BLUE));
        }

        for (uint32_t y = 0; y <= height; ++y)
        {
            mDDraw->DrawRectangle(ROUND(leftTop.X) * mCellSize + BOUNDARY, (y + ROUND(leftTop.Y)) * mCellSize + BOUNDARY,
                mCellSize - BOUNDARY, mCellSize - BOUNDARY,
                Color::ToARGBHex(colors::BLUE));
            mDDraw->DrawRectangle(ROUND(rightBottom.X) * mCellSize + BOUNDARY, (y + ROUND(leftTop.Y)) * mCellSize + BOUNDARY,
                mCellSize - BOUNDARY, mCellSize - BOUNDARY,
                Color::ToARGBHex(colors::BLUE));
        }
    }
}

void Canvas::DrawPixel(const Vector2& pos, const Color& color, const bool bCommand)
{
    if (!isValidPos(pos))
    {
        return;
    }

    if (bCommand && mCanvas != mBackCanvas)
    {
        std::vector<Vector2> pointsVect;
        pointsVect.push_back(pos);

        mPixelHistory[ROUND(pos.Y) * mCols + ROUND(pos.X)].push_back(GetPixel(pos));

        mRedoCommandHistory.clear();
        mRedoPixelHistory.clear();

        std::vector<Vector2> posVect;
        posVect.push_back(pos);
        mCommandHistory.push_back(std::move(posVect));

        mPointsHistory.push_back(std::move(pointsVect));
        mRedoPointsHistory.clear();
        mLastCommandType = eCommandType::Pixel;
    }

    mCanvas[ROUND(pos.Y) * mCols + ROUND(pos.X)] = color;
}

void Canvas::RemovePixel(const Vector2& pos, const bool bCommand)
{
    if (!isValidPos(pos))
    {
        return;
    }

    DrawPixel(pos, colors::WHITE, bCommand);

    if (!bCommand && mCanvas != mBackCanvas)
    {
        mLastCommandType = eCommandType::Remove;
    }
}

void Canvas::DrawLineDDA(const Vector2& startPos, const Vector2& endPos, const Color& color)
{
    std::vector<Vector2> posVect;
    Vector2 pos = startPos;

    float dx = endPos.X - startPos.X;
    float dy = endPos.Y - startPos.Y;
    float steps = (ABS(dx) > (ABS(dy))) ? ABS(dx) : ABS(dy);

    float xIncreament = dx / (float)steps;
    float yIncreament = dy / (float)steps;

    recordPosToHistory(startPos, &posVect);
    DrawPixel(startPos, color, false);

    for (uint32_t i = 0; i < steps; ++i)
    {
        pos.X += xIncreament;
        pos.Y += yIncreament;

        recordPosToHistory(pos, &posVect);
        DrawPixel(pos, color, false);
    }

    if (mCanvas != mBackCanvas)
    {
        std::vector<Vector2> pointsVect;
        pointsVect.push_back(startPos);
        pointsVect.push_back(endPos);

        mCommandHistory.push_back(std::move(posVect));

        mRedoCommandHistory.clear();
        mRedoPixelHistory.clear();

        mPointsHistory.push_back(std::move(pointsVect));
        mRedoPointsHistory.clear();
        mLastCommandType = eCommandType::LineDDA;
    }
}

void Canvas::DrawLineBresenham(const Vector2& startPos, const Vector2& endPos, const Color& color)
{
    std::vector<Vector2> posVect;

    float width = endPos.X - startPos.X;
    float height = endPos.Y - startPos.Y;
    bool bGradualSlope = (ABS(width) >= ABS(height)) ? true : false;

    float dx = (width >= 0.0f) ? 1.0f : -1.0f;
    float dy = (height >= 0.0f) ? 1.0f : -1.0f;
    float w = dx * width;
    float h = dy * height;

    float p = (bGradualSlope) ? 2.0f * h - w : 2.0f * w - h;
    float p1 = (bGradualSlope) ? 2.0f * h : 2.0f * w;
    float p2 = (bGradualSlope) ? 2.0f * h - 2.0f * w : 2.0f * w - 2.0f * h;

    Vector2 pos = startPos;

    if (bGradualSlope)
    {
        while (ROUND(pos.X) != ROUND(endPos.X))
        {
            recordPosToHistory(pos, &posVect);
            DrawPixel(pos, color, false);

            if (p < 0)
            {
                p += p1;
            }
            else
            {
                pos.Y += dy;
                p += p2;
            }

            pos.X += dx;
        }
    }
    else
    {
        while (ROUND(pos.Y) != ROUND(endPos.Y))
        {
            recordPosToHistory(pos, &posVect);
            DrawPixel(pos, color, false);

            if (p < 0)
            {
                p += p1;
            }
            else
            {
                pos.X += dx;
                p += p2;
            }

            pos.Y += dy;
        }
    }

    recordPosToHistory(pos, &posVect);
    DrawPixel(pos, color, false);

    if (mCanvas != mBackCanvas)
    {
        std::vector<Vector2> pointsVect;
        pointsVect.push_back(startPos);
        pointsVect.push_back(endPos);

        mCommandHistory.push_back(std::move(posVect));

        mRedoCommandHistory.clear();
        mRedoPixelHistory.clear();

        mPointsHistory.push_back(std::move(pointsVect));
        mRedoPointsHistory.clear();
        mLastCommandType = eCommandType::LineBresenham;
    }
}

void Canvas::DrawCircle(const Vector2& centerPos, const float radius, const Color& color)
{
    std::vector<Vector2> posVect;

    float x = 0;
    float y = (float)radius;
    float p = 1.0f - (float)radius;

    Vector2 p0(centerPos.X + (float)radius, centerPos.Y);
    Vector2 p1(centerPos.X - (float)radius, centerPos.Y);
    Vector2 p2(centerPos.X, centerPos.Y + (float)radius);
    Vector2 p3(centerPos.X, centerPos.Y - (float)radius);

    recordPosToHistory(p0, &posVect);
    recordPosToHistory(p1, &posVect);
    recordPosToHistory(p2, &posVect);
    recordPosToHistory(p3, &posVect);

    DrawPixel(p0, color, false);
    DrawPixel(p1, color, false);
    DrawPixel(p2, color, false);
    DrawPixel(p3, color, false);

    while (x < y)
    {
        ++x;

        if (p < 0)
        {
            p = p + 2.0f * (x + 1.0f);
        }
        else
        {
            --y;
            p = p + 2.0f * (x - y) + 1.0f;
        }

        Vector2 p0(centerPos.X + x, centerPos.Y + y);
        Vector2 p1(centerPos.X + y, centerPos.Y + x);
        Vector2 p2(centerPos.X + y, centerPos.Y - x);
        Vector2 p3(centerPos.X + x, centerPos.Y - y);
        Vector2 p4(centerPos.X - x, centerPos.Y - y);
        Vector2 p5(centerPos.X - y, centerPos.Y - x);
        Vector2 p6(centerPos.X - y, centerPos.Y + x);
        Vector2 p7(centerPos.X - x, centerPos.Y + y);

        recordPosToHistory(p0, &posVect);
        recordPosToHistory(p1, &posVect);
        recordPosToHistory(p2, &posVect);
        recordPosToHistory(p3, &posVect);
        recordPosToHistory(p4, &posVect);
        recordPosToHistory(p5, &posVect);
        recordPosToHistory(p6, &posVect);
        recordPosToHistory(p7, &posVect);

        DrawPixel(p0, color, false);
        DrawPixel(p1, color, false);
        DrawPixel(p2, color, false);
        DrawPixel(p3, color, false);
        DrawPixel(p4, color, false);
        DrawPixel(p5, color, false);
        DrawPixel(p6, color, false);
        DrawPixel(p7, color, false);
    }

    if (mCanvas != mBackCanvas)
    {
        std::vector<Vector2> pointsVect;
        pointsVect.push_back(centerPos);

        mCommandHistory.push_back(std::move(posVect));

        mRedoCommandHistory.clear();
        mRedoPixelHistory.clear();

        mPointsHistory.push_back(std::move(pointsVect));
        mRedoPointsHistory.clear();
        mRadius = radius;
    }
}

void Canvas::DrawTriangle(const Vector2& pos0, const Vector2& pos1, const Vector2& pos2, const Color& color)
{
    DrawLineBresenham(pos0, pos1, color);
    DrawLineBresenham(pos1, pos2, color);
    DrawLineBresenham(pos2, pos0, color);

    if (mCanvas != mBackCanvas)
    {
        std::vector<Vector2> pointsVect;
        pointsVect.push_back(pos0);
        pointsVect.push_back(pos1);
        pointsVect.push_back(pos2);

        std::vector<Vector2> posVect;
        for (uint32_t i = 0; i < 3; ++i)
        {
            std::vector<Vector2>& linePosVect = mCommandHistory.back();
            posVect.insert(posVect.end(), linePosVect.begin(), linePosVect.end());

            mCommandHistory.pop_back();
            mPointsHistory.pop_back();
        }

        mCommandHistory.push_back(std::move(posVect));

        mRedoCommandHistory.clear();
        mRedoPixelHistory.clear();

        mPointsHistory.push_back(std::move(pointsVect));
        mRedoPointsHistory.clear();
        mLastCommandType = eCommandType::Triangle;
    }
}

void Canvas::DrawRectangle(const Vector2& pos0, const Vector2& pos1, const Color& color)
{
    Vector2 leftTop(pos0);
    Vector2 rightBottom(pos1);
    Vector2 leftBottom(pos0.X, pos1.Y);
    Vector2 rightTop(pos1.X, pos0.Y);

    DrawLineBresenham(leftTop, rightTop, color);
    DrawLineBresenham(leftTop, leftBottom, color);
    DrawLineBresenham(rightTop, rightBottom, color);
    DrawLineBresenham(leftBottom, rightBottom, color);

    if (mCanvas != mBackCanvas)
    {
        std::vector<Vector2> pointsVect;
        pointsVect.push_back(pos0);
        pointsVect.push_back(pos1);

        std::vector<Vector2> posVect;
        for (uint32_t i = 0; i < 4; ++i)
        {
            std::vector<Vector2>& linePosVect = mCommandHistory.back();
            posVect.insert(posVect.end(), linePosVect.begin(), linePosVect.end());

            mCommandHistory.pop_back();
            mPointsHistory.pop_back();
        }

        mCommandHistory.push_back(std::move(posVect));

        mRedoCommandHistory.clear();
        mRedoPixelHistory.clear();

        mPointsHistory.push_back(std::move(pointsVect));
        mRedoPointsHistory.clear();
        mLastCommandType = eCommandType::Region;
    }
}

bool Canvas::Undo()
{
    if (CanUndo() && !mbClipped)
    {
        std::vector<Color> colorVect;

        for (Vector2& pos : mCommandHistory.back())
        {
            colorVect.push_back(GetPixel(pos));

            Color prevColor = mPixelHistory[ROUND(pos.Y) * mCols + ROUND(pos.X)].back();
            mPixelHistory[ROUND(pos.Y) * mCols + ROUND(pos.X)].pop_back();

            DrawPixel(pos, prevColor, false);
        }

        mRedoCommandHistory.push_back(mCommandHistory.back());
        mRedoPixelHistory.push_back(std::move(colorVect));
        mCommandHistory.pop_back();

        mRedoPointsHistory.push_back(mPointsHistory.back());
        mPointsHistory.pop_back();

        return true;
    }

    return false;
}

bool Canvas::Redo()
{
    if (CanRedo() && !mbClipped)
    {
        std::vector<Vector2>& redoCommandPosVect = mRedoCommandHistory.back();
        std::vector<Color>& redoPixelVect = mRedoPixelHistory.back();
        for (uint32_t i = 0; i < redoCommandPosVect.size(); ++i)
        {
            Vector2& pos = redoCommandPosVect[i];
            Color& color = redoPixelVect[i];

            mPixelHistory[ROUND(pos.Y) * mCols + ROUND(pos.X)].push_back(GetPixel(pos));

            DrawPixel(pos, color, false);
        }

        mCommandHistory.push_back(redoCommandPosVect);
        mRedoCommandHistory.pop_back();
        mRedoPixelHistory.pop_back();

        mPointsHistory.push_back(mRedoPointsHistory.back());
        mRedoPointsHistory.pop_back();

        return true;
    }

    return false;
}

BOOL CALLBACK Canvas::DialogEventHandler(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    switch (iMessage)
    {
    case WM_COMMAND:
        if (mhDialog == nullptr)
        {
            mhDialog = hDlg;
        }
        if (HIWORD(wParam) == BN_CLICKED)
        {
            switch (LOWORD(wParam))
            {
            case IDC_RADIO_PIXEL:
                mCurCommandType = eCommandType::Pixel;
                break;
            case IDC_RADIO_REMOVE:
                mCurCommandType = eCommandType::Remove;
                break;
            case IDC_RADIO_LINE_DDA:
                mCurCommandType = eCommandType::LineDDA;
                break;
            case IDC_RADIO_LINE_BRESENHAM:
                mCurCommandType = eCommandType::LineBresenham;
                break;
            case IDC_RADIO_CIRCLE:
                mCurCommandType = eCommandType::Circle;
                break;
            case IDC_RADIO_TRIANGLE:
                mCurCommandType = eCommandType::Triangle;
                break;
            case IDC_RADIO_REGION:
                mCurCommandType = eCommandType::Region;
                break;
            default:
                break;
            }
        }
        switch (LOWORD(wParam))
        {
        case IDC_BUTTON_UNDO:
            Undo();
            break;
        case IDC_BUTTON_REDO:
            Redo();
            break;
        case IDC_BUTTON_CLEAR:
            Clear();
            break;
        case IDC_BUTTON_TRANSLATE:
        {
            wchar_t dxText[128];
            wchar_t dyText[128];
            GetDlgItemText(hDlg, IDC_EDIT_TRANSLATION_X, dxText, 128);
            GetDlgItemText(hDlg, IDC_EDIT_TRANSLATION_Y, dyText, 128);

            int dx = _wtoi(dxText);
            int dy = _wtoi(dyText);
            dx = IsDlgButtonChecked(hDlg, IDC_CHECK_TRANSLATION_X_NEGATIVE) ? -dx : dx;
            dy = IsDlgButtonChecked(hDlg, IDC_CHECK_TRANSLATION_Y_NEGATIVE) ? -dy : dy;

            std::vector<Vector2> newPosVect;
            Transform::Translate(mPointsHistory.back(), dx, dy, &newPosVect);

            switch (mLastCommandType)
            {
            case eCommandType::Pixel:
                DrawPixel(newPosVect[0], colors::RED);
                break;
            case eCommandType::LineDDA:
                // fall-through
            case eCommandType::LineBresenham:
                DrawLineBresenham(newPosVect[0], newPosVect[1], colors::RED);
                break;
            case eCommandType::Circle:
                DrawCircle(newPosVect[0], mRadius, colors::RED);
                break;
            case eCommandType::Triangle:
                DrawTriangle(newPosVect[0], newPosVect[1], newPosVect[2], colors::RED);
                break;
            default:
                break;
            }

            break;
        }
        case IDC_BUTTON_SCALE:
        {
            wchar_t scalarXText[128];
            wchar_t scalarYText[128];
            GetDlgItemText(hDlg, IDC_EDIT_SCALING_X, scalarXText, 128);
            GetDlgItemText(hDlg, IDC_EDIT_SCALING_Y, scalarYText, 128);

            float scalarX = (float)_wtof(scalarXText);
            float scalarY = (float)_wtof(scalarYText);

            std::vector<Vector2> newPosVect;
            std::vector<Vector2> pointsVect = mPointsHistory.back();

            switch (mLastCommandType)
            {
            case eCommandType::Pixel:
                pointsVect.push_back(pointsVect.back());
                // fall-through
            case eCommandType::LineDDA:
                // fall-through
            case eCommandType::LineBresenham:
            {
                Vector2 fixedPos((pointsVect[0].X + pointsVect[1].X) / 2.0f,
                    (pointsVect[0].Y + pointsVect[1].Y) / 2.0f);
                Transform::Scale(pointsVect, Vector2(scalarX, scalarY), fixedPos, &newPosVect);
                DrawLineBresenham(newPosVect[0], newPosVect[1], colors::RED);
                break;
            }
            case eCommandType::Circle:
                Transform::Scale(pointsVect, Vector2(scalarX, scalarY), pointsVect[0], &newPosVect);
                DrawCircle(newPosVect[0], mRadius * scalarX, colors::RED);
                break;
            case eCommandType::Triangle:
            {
                Vector2 fixedPos((pointsVect[0].X + pointsVect[1].X + pointsVect[2].X) / 3.0f,
                    (pointsVect[0].Y + pointsVect[1].Y + pointsVect[2].Y) / 3.0f);
                Transform::Scale(pointsVect, Vector2(scalarX, scalarY), fixedPos, &newPosVect);
                DrawTriangle(newPosVect[0], newPosVect[1], newPosVect[2], colors::RED);
                break;
            }
            default:
                break;
            }

            break;
        }
        case IDC_BUTTON_ROTATE:
        {
            wchar_t sinXText[128];
            wchar_t cosXText[128];
            GetDlgItemText(hDlg, IDC_EDIT_ROTATION_SINX, sinXText, 128);
            GetDlgItemText(hDlg, IDC_EDIT_ROTATION_COSX, cosXText, 128);

            float sinX = (float)_wtof(sinXText);
            float cosX = (float)_wtof(cosXText);

            std::vector<Vector2> newPosVect;
            std::vector<Vector2> pointsVect = mPointsHistory.back();

            switch (mLastCommandType)
            {
            case eCommandType::LineDDA:
            // fall-through
            case eCommandType::LineBresenham:
            {
                Vector2 pivotPos((pointsVect[0].X + pointsVect[1].X) / 2.0f,
                    (pointsVect[0].Y + pointsVect[1].Y) / 2.0f);
                Transform::Rotate(pointsVect, Vector2(sinX, cosX), pivotPos, &newPosVect);
                DrawLineBresenham(newPosVect[0], newPosVect[1], colors::RED);
                break;
            }
            case eCommandType::Triangle:
            {
                Vector2 pivotPos((pointsVect[0].X + pointsVect[1].X + pointsVect[2].X) / 3.0f,
                    (pointsVect[0].Y + pointsVect[1].Y + pointsVect[2].Y) / 3.0f);
                Transform::Rotate(pointsVect, Vector2(sinX, cosX), pivotPos, &newPosVect);
                DrawTriangle(newPosVect[0], newPosVect[1], newPosVect[2], colors::RED);
                break;
            }
            default:
                break;
            }

            break;
        }
        case IDC_BUTTON_CLEAR_REGION:
        {
            mbClipped = false;
            mbClippingRegion = false;
            mCanvas = mFrontCanvas;
            Button_SetCheck(GetDlgItem(hDlg, IDC_CHECK_CLIP), BST_UNCHECKED);
            break;
        }
        default:
            break;
        }
        break;
    }

    return FALSE;
}

void Canvas::windowToViewport(const Vector2& pos, Vector2* outPos)
{
    AssertW(mClippingRegionLeftTopPos.X != -1, L"No clipping region");
    AssertW(mClippingRegionRightBottomPos.X != -1, L"No clipping region");
    AssertW(outPos != nullptr, L"outPos is nullptr");

    const Vector2& windowLeftTopPos = mClippingRegionLeftTopPos;
    const Vector2& windowRightTopPos = mClippingRegionRightBottomPos;

    float sx = (float)mCols / (windowRightTopPos.X - windowLeftTopPos.X);
    float sy = (float)mRows / (windowRightTopPos.Y - windowLeftTopPos.Y);

    const Vector2 posInWindow(pos.X - windowLeftTopPos.X, pos.Y - windowLeftTopPos.Y);

    const float m0[3][3] =
    {
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
        { posInWindow.X, posInWindow.Y, 1.0f }
    };
    const float m1[3][3] =
    {
        { sx, 0.0f, 0.0f},
        { 0.0f, sy, 0.0f},
        { 0.0f, 0.0f, 1.0f}
    };
    const float m2[3] = { posInWindow.X, posInWindow.Y, 1.0f };

    float tempMatrix[3][3] = {};
    float resultMatrix[3] = {};

    Matrix::MulMatrix33x33(m0, m1, tempMatrix);
    Matrix::MulMatrix13x33(m2, tempMatrix, resultMatrix);

    outPos->X = resultMatrix[0];
    outPos->Y = resultMatrix[1];
}

static bool ClipCohem(Vector2* inOutStartPos, Vector2* inOutEndPos, const Vector2& regionLeftTopPos, const Vector2& regionRightBottomPos)
{
    AssertW(inOutStartPos != nullptr, L"inOutStartPos is nullptr");
    AssertW(inOutEndPos != nullptr, L"inOutEndPos is nullptr");

    uint8_t startCode = regioncode::MakeRegionCode(*inOutStartPos, regionLeftTopPos, regionRightBottomPos);
    uint8_t endCode = regioncode::MakeRegionCode(*inOutEndPos, regionLeftTopPos, regionRightBottomPos);

    while (true)
    {
        uint8_t outsideCode = (startCode == 0) ? endCode : startCode;

        if ((startCode == 0) && (endCode == 0))
        {
            return true;
        }
        else if (startCode & endCode)
        {
            return false;
        }
        else
        {
            float width = inOutEndPos->X - inOutStartPos->X;
            float height = inOutEndPos->Y - inOutStartPos->Y;
            Vector2 pos;

            // 오른쪽 또는 왼쪽인 경우
            if (outsideCode < 0b0100)
            {
                if (outsideCode & 0b0010)
                {
                    pos.X = regionRightBottomPos.X;
                }
                else
                {
                    pos.X = regionLeftTopPos.X;
                }

                pos.Y = inOutStartPos->Y + height * (pos.X - inOutStartPos->X) / width;
            }
            else // 위쪽 또는 아래쪽인 경우
            {
                if (outsideCode & 0b1000)
                {
                    pos.Y = regionLeftTopPos.Y;
                }
                else
                {
                    pos.Y = regionRightBottomPos.Y;
                }

                pos.X = inOutStartPos->X + width * (pos.Y - inOutStartPos->Y) / height;
            }

            if (startCode == 0)
            {
                *inOutEndPos = pos;
                endCode = regioncode::MakeRegionCode(*inOutEndPos, regionLeftTopPos, regionRightBottomPos);
            }
            else
            {
                *inOutStartPos = pos;
                startCode = regioncode::MakeRegionCode(*inOutStartPos, regionLeftTopPos, regionRightBottomPos);
            }
        }
    }
}

static bool ClipLiang(Vector2* inOutStartPos, Vector2* inOutEndPos, const Vector2& regionLeftTopPos, const Vector2& regionRightBottomPos)
{
    float width = inOutEndPos->X - inOutStartPos->X;
    float height = inOutEndPos->Y - inOutStartPos->Y;

    const float p[4] = { -width, width, -height, height };
    const float q[4] =
    {
        inOutStartPos->X - regionLeftTopPos.X,
        regionRightBottomPos.X - inOutEndPos->X,
        inOutStartPos->Y - regionLeftTopPos.Y,
        regionRightBottomPos.Y - inOutStartPos->Y
    };
    const float u[4] =
    {
        q[0] / p[0],
        q[1] / p[1],
        q[2] / p[2],
        q[3] / p[3]
    };

    float u0 = MIN(ABS(u[0]), ABS(u[2]));
    float u1 = MIN(ABS(u[1]), ABS(u[3]));

    if (u0 > u1)
    {
        return false;
    }
    else
    {
        if (u0 > u1)
        {
            return false;
        }

        inOutStartPos->X = inOutStartPos->X + width * u0;
        inOutStartPos->Y = inOutStartPos->Y + height * u1;

        return true;
    }
}