#pragma once

class Transform final
{
public:
	Transform() = delete;
	Transform(const Transform&) = delete;
	Transform(const Transform&&) = delete;
	Transform& operator=(const Transform&) = delete;
	~Transform() = default;

	static void Translate(const std::vector<Vector2>& points, const int32_t dx, const int32_t dy, std::vector<Vector2>* outPoints);
	static void Scale(const std::vector<Vector2>& points, const Vector2& scalar, const Vector2& fixedPos, std::vector<Vector2>* outPoints);
	static void Rotate(const std::vector<Vector2>& points, const Vector2& sinCosX, const Vector2& pivotPos, std::vector<Vector2>* outPoints);
};