#include "CharacterBase.h"
#include "CharacterAnimInstance.h"


ACharacterBase::ACharacterBase(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UBaseMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AttributesComponent = CreateDefaultSubobject<UAttributesComponent>("AttributesComponent");
	LevelComponent = CreateDefaultSubobject<ULevelComponent>("LevelComponent");
	CombatComponent = CreateDefaultSubobject<UCombatComponent>("CombatComponent");
	EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>("EquipmentComponent");
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("InventoryComponent");
	CurrentCCState = ECCState::NoCC;
	MaxWeightMod = 10.f;
}

UBaseMovementComponent* ACharacterBase::GetMovementComponent() const
{
	return Cast<UBaseMovementComponent>(ACharacter::GetMovementComponent());
}

UCombatComponent* ACharacterBase::GetCombatComponent() const
{
	check(CombatComponent != nullptr && "No CombatComponent")
	return CombatComponent;
}

UAttributesComponent* ACharacterBase::GetAttributesComponent() const
{	
	check(AttributesComponent != nullptr && "No AttrComponent")
	return AttributesComponent;
}

ULevelComponent* ACharacterBase::GetLevelComponent() const
{	
	check(LevelComponent != nullptr && "No LevelComponent")
	return LevelComponent;
}

UEquipmentComponent* ACharacterBase::GetEquipmentComponent() const
{
	check(EquipmentComponent != nullptr && "No EquipmentComponent")
	return EquipmentComponent;
}

UInventoryComponent* ACharacterBase::GetInventoryComponent() const
{
	check(InventoryComponent != nullptr && "No InventoryComponent")
	return InventoryComponent;
}

float ACharacterBase::TakeDamage(float DamageAmount,
								const struct FDamageEvent &DamageEvent,
								class AController *EventInstigator,
								AActor *DamageCauser)
{
	const auto DamageType = DamageEvent.DamageTypeClass.GetDefaultObject();

	const auto PhysD = Cast<UPhysicDamage>(DamageType);
	const auto MagicD = Cast<UMagicDamage>(DamageType);

	if (PhysD != nullptr)
	{
		auto Type = PhysD->Type;
		auto Force = PhysD->Force;
		DamageAmount -= GetAttributesComponent()->CurrentDamageAbsorb.DamageMap[(int32)Type];
		DamageAmount = DamageAmount * 
			(GetAttributesComponent()->CurrentDamageResistance.DamageMap[(int32)Type] - 100) / 100;
		GetAttributesComponent()->Health->Consume(DamageAmount);// -> bool
		//TODO: Death, Force, DamageItem
		return DamageAmount;
	}
	else if (MagicD != nullptr)
	{// тоже что и физический + проверка на пробитие магического щита если такой имеется
		auto Type = MagicD->Type;
		auto MagicSchool = MagicD->MagicSchool;
		DamageAmount -= GetAttributesComponent()->CurrentDamageAbsorb.DamageMap[(int32)Type];
		DamageAmount = DamageAmount * 
			(GetAttributesComponent()->CurrentDamageResistance.DamageMap[(int32)Type] - 100) / 100;
		if(DamageAmount < 0)
		{
			if(GetAttributesComponent()->Health->Consume(DamageAmount))
				return DamageAmount = 0;
			else
				return DamageAmount = DamageAmount - GetAttributesComponent()->MagicShield->GetValue();
		}
		return DamageAmount = 0;
	}
	else if (DamageType) //if damage nor Physical or Magical, then itВґs raw Damage
	{
		GetAttributesComponent()->Health->Consume(-DamageAmount); // -> bool
		return DamageAmount;
	}

	return 0;
}
void ACharacterBase::OnJumped_Implementation()
{
	GetMovementComponent()->bCanHangAgain = false;
	GetMovementComponent()->bWasJustJumping = true;
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();	
}

void ACharacterBase::OnUpdate()
{
	this->Update();
}

void ACharacterBase::Update()
{
	if(GetLevelComponent()->IsUpdateNeeded())
		GetLevelComponent()->Update();
	if(GetAttributesComponent()->IsUpdateNeeded())
	{
		GetAttributesComponent()->Update();
		CalculateMaxWeight();
	}
	if(GetEquipmentComponent()->IsUpdateNeeded())
		GetEquipmentComponent()->Update();
	if(GetInventoryComponent()->IsUpdateNeeded())
		GetInventoryComponent()->Update();
	if(GetMovementComponent()->IsUpdateNeeded())		
		GetMovementComponent()->Update();

	//other calculation
}

void ACharacterBase::DoLevelUp(const FLevelInfo& levelInfo)
{
	GetLevelComponent()->LevelUp(levelInfo);
	GetLevelComponent()->SetUpdate();
	GetAttributesComponent()->SetUpdate();
	bool dexChanged = UAttributesHelper::GetAttribute(levelInfo.Attributes, EAttributes::Dex) != 0;
	bool strChanged = UAttributesHelper::GetAttribute(levelInfo.Attributes, EAttributes::Str) != 0;
	bool acrobaticsChanged = USkillHelper::GetValue(levelInfo.SkillList, ESkill::Tumble) != 0;
	if(strChanged)
	{
		GetInventoryComponent()->SetUpdate();
		GetEquipmentComponent()->SetUpdate();
	}
	if(dexChanged || strChanged || acrobaticsChanged)
		GetMovementComponent()->SetUpdate();
	// может что то еще
	Update();
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACharacterBase::CalculateMaxWeight()
{
	const int32 str =	UAttributesHelper::GetAttribute(GetAttributesComponent()->CurrentAttributes, EAttributes::Str);
	MaxWeight = MaxWeightMod * str;	// clever formula!
}

void ACharacterBase::StartAnimation(UAnimMontage* AnimMontage, AnimTypes Type, float InPlayRate, FName StartSectionName)
{
	PlayAnimMontage(AnimMontage, InPlayRate, StartSectionName);
	// todo add delegates according to anim type
	auto animInstance = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());
	auto montageInstance = animInstance->GetActiveInstanceForMontage(AnimMontage);
	switch(Type)
	{
		case AnimTypes::Block:
		{
			break;
		}
		case AnimTypes::Parkour:
		{
			montageInstance->OnMontageBlendingOutStarted.BindUObject(GetMovementComponent(), 
				&UBaseMovementComponent::ParkourInterrupt);		
			break;
		}
		case AnimTypes::SpellCast:
		{
			break;
		}
		case AnimTypes::Attack:
		{
			break;
		}
		case AnimTypes::Dodge:
		{
			montageInstance->OnMontageBlendingOutStarted.BindUObject(GetMovementComponent(), 
				&UBaseMovementComponent::DodgeInterrupt);		
			break;
		}
		case AnimTypes::CC:
		{
			break;
		}
		default:
			check(0 && "Start Anim: invalid anim type")
	}
}

void ACharacterBase::InterruptAnimation(ECCState Source, int OldState)
{
	//todo

	// already controlled
	if(OldState != ECCState::NoCC)
	{
	//check if highest order bit of source is lower than old
	#ifdef PLATFORM_WINDOWS
		unsigned long sourceIndex;
		unsigned long oldIndex;
		_BitScanReverse(&sourceIndex, Source);
		_BitScanReverse(&oldIndex, OldState);
		if (sourceIndex < oldIndex)
	#else
		if (ffs(Source) < ffs(OldState))
	#endif
		{
			UE_LOG(LogTemp,
			       Warning,
			       TEXT("InterruptAnimation: cant interrupt CC, low priority"))
			return;
		}
		// todo replace old CC anim with New via the powerfull switch(Source){}
		//StartAnimation(Source, someCCAnim, 1)
	}
	else
	{
		///// todo determine character state (combat, movement, transition, )
		// interrupted by character
		// player can interrupt blocks, spell casts, wall hanging, some attacks and not much else
		if(Source == ECCState::NoCC)
		{
			if(GetMovementComponent()->MovementMode == EMovementMode::MOVE_Custom)
			{
				auto animInstance = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());
				if(animInstance->Montage_IsActive(GetMovementComponent()->HangMontage))
					StopAnimMontage(GetMovementComponent()->HangMontage);
			}
			StopAnimMontage(GetMovementComponent()->VaultMontageHigh);	// todo temp debug
			return;
		}
		// interrupted by CC
		if(GetMovementComponent()->IsTransitionInProgress())
		{
			GetMovementComponent()->TransitionEnd();
			return;
		}
		if(GetMovementComponent()->MovementMode == EMovementMode::MOVE_Custom)
		{
			auto animInstance = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());
			switch (GetMovementComponent()->CustomMovementMode)
			{
					case ECustomMovementMode::ClimbingWall:
					{
						UAnimMontage* wallClimbAnims[] = {GetMovementComponent()->ClimbWallCrouchedMontage, 
														GetMovementComponent()->ClimbWallMontage, 
														GetMovementComponent()->ClimbDownMontage, 
														GetMovementComponent()->HangMontage};
						for(int i = 0; i < 4; ++i)
						{
							if(animInstance->Montage_IsActive(wallClimbAnims[i]))
								StopAnimMontage(wallClimbAnims[i]);
						}
						break;
					}
					case ECustomMovementMode::ClimbingLedge:
					{
						UAnimMontage* ledgeClimbAnims[] = {GetMovementComponent()->VaultMontageLow, 
														GetMovementComponent()->VaultMontageHigh, 
														GetMovementComponent()->ClimbLedgeMontageLow, 
														GetMovementComponent()->ClimbLedgeMontageHigh,
														GetMovementComponent()->ClimbLedgeCrouchedMontageLow,
														GetMovementComponent()->ClimbLedgeCrouchedMontageHigh};
						for(int i = 0; i < 6; ++i)
						{
							if(animInstance->Montage_IsActive(ledgeClimbAnims[i]))
								StopAnimMontage(ledgeClimbAnims[i]);
						}
						break;
					}				
					case ECustomMovementMode::Dodge: // dodge is implemented via transition
						break;
					default:
						check(0 && "interrupt Anim: invalid move mode")
			}
		}
	}
}

void ACharacterBase::InterruptAction(ECCState Source)
{
	// todo handle states and ...
	InterruptAnimation(Source, CurrentCCState);
	CurrentCCState |= Source;
}
