#pragma once

#include "Kismet/KismetMathLibrary.h"
#include "Components/SceneComponent.h"
//////////////////// Файл для вспомогательных общих функцих, формул и.т.д //////////////

namespace MadFireUtils
{
// экспонента ограниченная между lowerbound и upperbound, возрастающая(при отрицательных x убывает)
// при х = 0, y = UpperBound / 2 + LowerBound(но это не точно)
// mod - чем больше тем быстрее наступает максимум/мин, чем меньше = тем линейней
// по умолчанию обычная сигмоида с y= от 0 до 1
// todo добавить расчет смещения нуля функции
// todo упростить задание пределов и точки y при х == 0
static float GetSigmoidValue(float x, float mod = 1, float lowerBound = 0, float upperBound = 1)
{
    const float epsilon = 0.001f;
    check(upperBound - lowerBound > epsilon && "Bad Arguments");
    const float dividend = upperBound - lowerBound;
    return dividend / (FMath::Exp(-(FMath::Abs(mod) * x)) + 1.f) + lowerBound;
}

// Нахождение сокета на модели, который ближе всего в координатах мира к заданной точке
// Возвращает None FName если не нашел сокетов
static FName GetNearestSocketToLocation(USceneComponent* Mesh, FVector Location)
{
    FName NearestSocketName;
    double NearestSocketDistance = std::numeric_limits<double>::max();
    auto Sockets = Mesh->GetAllSocketNames();
    for (auto& Name : Sockets)
    {
        FVector SocketLoc = Mesh->GetSocketLocation(Name);
        SocketLoc -= Location;
        double Distance = SocketLoc.Length();
        if (NearestSocketName.IsNone() || Distance < NearestSocketDistance)
        {
            NearestSocketName = Name;
            NearestSocketDistance = Distance;
        }
    }
    return NearestSocketName;
}
};