#include "FSMGraphFactory.h"
#include "FSMGraph.h"


UFSMGraphAssetFactory::UFSMGraphAssetFactory(const FObjectInitializer &ObjectInitializer) :
    Super(ObjectInitializer)
{
	bCreateNew = true;
    bEditAfterNew = true;
    SupportedClass = UFSMGraph::StaticClass();
}

UObject* UFSMGraphAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    return NewObject<UObject>(InParent, Class, Name, Flags | RF_Transactional);
}

bool UFSMGraphAssetFactory::CanCreateNew() const 
{
    return true;
}

bool UFSMGraphAssetFactory::ShouldShowInNewMenu() const
{
    return true;
}