#include "AbstractRequirement.h"

FText UAbstractRequirement::GetStringOfRequirement() 
{
	SetDescriptionParameters();
	return FText::Format(RequirementDescription, Arguments);
}

bool UAndRequirement::CheckRequirement_Implementation(ACharacter *character)
{
	if (!character)
		return false;

	if (!RequirementLHS || !RequirementRHS)
		return false;

	return RequirementLHS->CheckRequirement(character) && RequirementRHS->CheckRequirement(character);
}

void UAndRequirement::SetDescriptionParameters()
{	
	if (!RequirementLHS || !RequirementRHS)
		return;
	// Возмножно имеет смысл заменить это RequirementLHS на более которкие LHS
	// С другой стороны хотелось бы чтобы Названия аргументов соответствоали
	// Названиям переменных
	Arguments.Add(TEXT("RequirementLHS"), RequirementLHS->GetStringOfRequirement());
	Arguments.Add(TEXT("RequirementRHS"), RequirementRHS->GetStringOfRequirement());
}

bool UOrRequirement::CheckRequirement_Implementation(ACharacter *character)
{
	if (!character)
		return false;

	if (!RequirementLHS || !RequirementRHS)
		return false;

	return RequirementLHS->CheckRequirement(character) || RequirementRHS->CheckRequirement(character);
}

void UOrRequirement::SetDescriptionParameters()
{
	if (!RequirementLHS || !RequirementRHS)
		return;
	Arguments.Add(TEXT("RequirementLHS"), RequirementLHS->GetStringOfRequirement());
	Arguments.Add(TEXT("RequirementRHS"), RequirementRHS->GetStringOfRequirement());
}

//Тут я решил проверить скорость персонажа, потому как в этом проекте у персонажа нету характиристек
bool UTestRequirement::CheckRequirement_Implementation(ACharacter *character)
{
	if (!character) //Если внезапно персонаж не задан, то он автоматически не проходит данную проверку
		return false;
	
	//Возвращаем результат проверки (Да, проверка скорости выполняется таким тупым способом)
	return (FVector::DotProduct(character->GetVelocity(),character->GetActorRotation().Vector()) > CheckableSpeed);
}

void UTestRequirement::SetDescriptionParameters()
{
	//Тут самое прикольное.
	//Мы добавляем параметр в аргументы
	//И дальше в описании он будет ставить значение этого параметра вместо
	//{CheckableSpeed}, что довольно круто...
	Arguments.Add(TEXT("CheckableSpeed"), FText::AsNumber(CheckableSpeed));
}

//Тут я решил проверить скорость персонажа, потому как в этом проекте у персонажа нету характиристек
bool UTestRequirementTwo::CheckRequirement_Implementation(ACharacter *character)
{
	if (!character) //Если внезапно персонаж не задан, то он автоматически не проходит данную проверку
		return false;

	//Возвращаем результат проверки (Да, проверка скорости выполняется таким тупым способом)
	return (FVector::DotProduct(character->GetVelocity(), character->GetActorRotation().Vector()) < CheckableSpeed);
}