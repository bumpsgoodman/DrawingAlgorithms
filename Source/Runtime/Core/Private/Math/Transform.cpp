#include "Precompiled.h"

void Transform::Translate(const std::vector<Vector2>& points, const int32_t dx, const int32_t dy, std::vector<Vector2>* outPoints)
{
	const float TRANSFORM_MATRIX[3][3] =
	{
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ (float)dx, (float)dy, 1.0f}
	};

	for (uint32_t k = 0; k < points.size(); ++k)
	{
		float originMatrix[3] = { points[k].X, points[k].Y, 1.0f };
		float newMatrix[3] = {};

		Matrix::MulMatrix13x33(originMatrix, TRANSFORM_MATRIX, newMatrix);

		outPoints->push_back(Vector2(newMatrix[0], newMatrix[1]));
	}
}

void Transform::Scale(const std::vector<Vector2>& points, const Vector2& scalar, const Vector2& fixedPos, std::vector<Vector2>* outPoints)
{
	const float TRANSFORM_MATRIX[3][3] =
	{
		{ scalar.X, 0.0f, 0.0f },
		{ 0.0f, scalar.Y, 0.0f },
		{ (1.0f - scalar.X) * fixedPos.X, (1.0f - scalar.Y) * fixedPos.Y, 1}
	};

	for (uint32_t k = 0; k < points.size(); ++k)
	{
		float originMatrix[3] = { points[k].X, points[k].Y, 1.0f };
		float newMatrix[3] = {};

		Matrix::MulMatrix13x33(originMatrix, TRANSFORM_MATRIX, newMatrix);

		outPoints->push_back(Vector2(newMatrix[0], newMatrix[1]));
	}
}

void Transform::Rotate(const std::vector<Vector2>& points, const Vector2& sinCosX, const Vector2& pivotPos, std::vector<Vector2>* outPoints)
{
	const float TRANSFORM_MATRIX[3][3] =
	{
		{ sinCosX.XY[1], sinCosX.XY[0], 0.0f},
		{ -sinCosX.XY[0], sinCosX.XY[1], 0.0f},
		{ (1.0f - sinCosX.XY[1]) * pivotPos.X + pivotPos.Y * sinCosX.XY[0], (1.0f - sinCosX.XY[1]) * pivotPos.Y - pivotPos.X * sinCosX.XY[0], 1.0f}
	};

	for (uint32_t k = 0; k < points.size(); ++k)
	{
		float originMatrix[3] = { points[k].X, points[k].Y, 1 };
		float newMatrix[3] = {};

		Matrix::MulMatrix13x33(originMatrix, TRANSFORM_MATRIX, newMatrix);

		outPoints->push_back(Vector2(newMatrix[0], newMatrix[1]));
	}
}