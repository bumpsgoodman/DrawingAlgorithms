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

void Matrix::MulMatrix33x33(const float matrix33_0[][3], const float matrix33_1[][3], float outMatrix33[][3])
{
	for (uint32_t i = 0; i < 3; i++)
	{
		for (uint32_t j = 0; j < 3; j++)
		{
			for (uint32_t k = 0; k < 3; k++)
			{
				outMatrix33[i][j] += matrix33_0[i][k] * matrix33_1[k][j];
			}
		}
	}
}