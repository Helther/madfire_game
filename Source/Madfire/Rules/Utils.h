#pragma once

#include "Kismet/KismetMathLibrary.h"

//////////////////// Файл для вспомогательных общих функцих, формул и.т.д //////////////

// экспонента ограниченная между lowerbound и upperbound, возрастающая(при отрицательных x убывает)
// при х = 0, y = UpperBound / 2 + LowerBound(но это не точно) 
// mod - чем больше тем быстрее наступает максимум/мин, чем меньше = тем линейней
// по умолчанию обычная сигмоида с y= от 0 до 1
// todo добавить расчет смещения нуля функции
// todo упростить задание пределов и точки y при х == 0
static float GetSigmoidValue(float x, float mod = 1, float lowerBound = 0, float upperBound = 1)
{
	const float epsilon = 0.001f;
	if(upperBound - lowerBound < epsilon)
		return 0.f;
	check(upperBound - lowerBound > 0 && "Bad Arguments")
	const float dividend = upperBound - lowerBound;
	return dividend / (FMath::Exp(-(FMath::Abs(mod) * x)) + 1.f) + lowerBound;
}
