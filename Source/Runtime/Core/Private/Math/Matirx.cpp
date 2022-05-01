#include "Precompiled.h"

void Matrix::MulMatrix13x33(const float matrix13[], const float matrix33[][3], float outMatrix13[])
{
	for (uint32_t i = 0; i < 3; ++i)
	{
		for (uint32_t j = 0; j < 3; ++j)
		{
			outMatrix13[i] += matrix13[j] * matrix33[j][i];
		}
	}
}