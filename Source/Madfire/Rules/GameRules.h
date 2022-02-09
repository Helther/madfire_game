// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/UserDefinedEnum.h"

#include "GameRules.generated.h"
// clang-format off
/**
 * 
 */
    UENUM(Meta = (Bitflags))
    enum class EMagicSchool
    {
        No				UMETA(DisplayName = "No"),
        Abjuration		UMETA(DisplayName = "Abjuration"),
        Conjuration		UMETA(DisplayName = "Conjeration"),
        Divination		UMETA(DisplayName = "Divination"),
        Enchantment		UMETA(DisplayName = "Enchantment"),
        Evocation		UMETA(DisplayName = "Evocation"),
        Illusion		UMETA(DisplayName = "Illusion"),
        Necromancy		UMETA(DisplayName = "Necromancy"),
        Transmutation	UMETA(DisplayName = "Transmutation"),
        Universal		UMETA(DisplayName = "Universal")
    };

    UENUM(BlueprintType)
    enum class EItemMaterial : uint8
    {
        None				UMETA(DisplayName = "None"),
        Wood				UMETA(DisplayName = "Abjuration"),
        Steel				UMETA(DisplayName = "Steel"),
        Silver				UMETA(DisplayName = "Silver"),
        Adamantine			UMETA(DisplayName = "Adamantine"),
        Darkwood			UMETA(DisplayName = "Darkwood"),
        ColdIron			UMETA(DisplayName = "ColdIron"),
        AlchemicalSilver	UMETA(DisplayName = "AlchemicalSilver"),
        Mithril				UMETA(DisplayName = "Mithril"),
        Glass				UMETA(DisplayName = "Glass")
        //Какая_то_неведомая_хрень
    };


    //enum EStats      
    UENUM(BlueprintType)
    enum class EAttributes : uint8
    {
        Str = 0 	UMETA(DisplayName = "Strength"),	//  Strength;        //Styrkur          //0
        Dex = 1		UMETA(DisplayName = "Dexterity"),	//  Dexterity;       //Ловкость         //1
        Con = 2		UMETA(DisplayName = "Constitution"),//  Constitution;    //Эээ              //2
        Int = 3		UMETA(DisplayName = "Intelligence"),//  Intelligence;    //Интилэкт         //3
        Wis = 4		UMETA(DisplayName = "Wisdom"),		//  Wisdom;          //Viska            //4
        Cha = 5		UMETA(DisplayName = "Charisma"),	//  Charisma;        //Persónutöfrar    //5
        Max			UMETA(Hidden)
    };


    //TODO: Обновить!
    UENUM(BlueprintType)
    enum class ESkill : uint8   //Название Скилов (Модификатор)
    {
        Appraise = 0		UMETA(DisplayName = "Appraise"),		//Оценка (Int)
        Knowledge   		UMETA(DisplayName = "Knowledge"),		//Знание (Int)
        Alchemy     		UMETA(DisplayName = "Alchemy"),			//Алхимия (Int)
        Enchantment     	UMETA(DisplayName = "Enchantment"),		//Зачарование предметов (Int)

        Speechcraft     	UMETA(DisplayName = "Speechcraft"),		//Красноречие  (Cha)
        Perform         	UMETA(DisplayName = "Perform"),			//Представление (Cha)
        HandleAnimal       	UMETA(DisplayName = "HandleAnimal"),	//Сочуствие животным (Cha)

        Perception  		UMETA(DisplayName = "Perception"),		//Восприятие (Wis)
        Listen      		UMETA(DisplayName = "Listen"),			//Слух (Wis)
        Survival    		UMETA(DisplayName = "Survival"),		//Выживание (Wis)

        SleightOfHand      	UMETA(DisplayName = "SleightOfHand"),	//Ловкость рук (Dex)
        OpenLock       		UMETA(DisplayName = "OpenLock"),		//Взлом замков (Dex)
        Sneakiness      	UMETA(DisplayName = "Sneakiness"),		//Скрытность (Dex)
        MoveSilently    	UMETA(DisplayName = "MoveSilently"),	//Скрытное перемещение (Dex)
        Balance         	UMETA(DisplayName = "Balance"),			//Баланс (Dex)
        Tumble          	UMETA(DisplayName = "Tumble"),			//Акробатика (Dex)

        Concentration   	UMETA(DisplayName = "Concentration"),	//Концентрация (Con)
        Blacksmithing   	UMETA(DisplayName = "Blacksmithing"),	//Кузнечество (Str)
    
        Max					UMETA(Hidden)
    };

    UENUM(BlueprintType)
    enum class EAmmoType : uint8
    {
        Bolt			UMETA(DisplayName = "Bolt"),
        Arrow			UMETA(DisplayName = "Arrow"),
        SomethingElse	UMETA(DisplayName = "Something"),
    };

    UENUM(BlueprintType)
    enum class EDamageType : uint8
    {
        Slashing		UMETA(DisplayName = "Slashing"),
        Bludgeoning		UMETA(DisplayName = "Bludgeoning"),          //Туповатый
        Piercing		UMETA(DisplayName = "Piercing"),
        Sonic			UMETA(DisplayName = "Sonic"),
        Fire			UMETA(DisplayName = "Fire"),
        Cold			UMETA(DisplayName = "Cold"),
        Lightning		UMETA(DisplayName = "Lightning"),
        Acid			UMETA(DisplayName = "Acid"),
        NegativeEnergy	UMETA(DisplayName = "NegativeEnergy"),
        PositiveEnergy	UMETA(DisplayName = "PositiveEnergy"),
        Max				UMETA(Hidden)
    };


    //Weapon attack modifiers
    UENUM(BlueprintType)
    enum class EWeaponSize : uint8 
    {
        Diminutive	= 0		UMETA(DisplayName = "Diminutive"),
        Tiny		= 2		UMETA(DisplayName = "Tiny"),
        Small		= 4		UMETA(DisplayName = "Small"),
        Medium		= 6		UMETA(DisplayName = "Medium"),
        Large		= 8		UMETA(DisplayName = "Large"),
        Huge		= 10	UMETA(DisplayName = "Huge"),
        Gargantuan	= 12	UMETA(DisplayName = "Gargantum"),
    };


    UENUM(BlueprintType)
    enum class EWeaponType : uint8
    {
        Dagger		UMETA(DisplayName = "Dagger"),
        Mace		UMETA(DisplayName = "Mace"),
        Sickle		UMETA(DisplayName = "Sickle"),
        Club		UMETA(DisplayName = "Club"),
        Morningstar UMETA(DisplayName = "Morningstar"),
        Shortspear	UMETA(DisplayName = "Shortspear"),
        Handaxe		UMETA(DisplayName = "Handaxe"),
        Quaterstaff UMETA(DisplayName = "Quaterstaff"),
        Spear		UMETA(DisplayName = "Spear"),
        Crossbow	UMETA(DisplayName = "Crossbow"),
        Dart		UMETA(DisplayName = "Dart"),
        Battleaxe	UMETA(DisplayName = "Battleaxe"),
        Longsword	UMETA(DisplayName = "Longsword"),
        Scimitar	UMETA(DisplayName = "Scimitar"),
        Warhammer	UMETA(DisplayName = "Warhammer"),
        Falchion	UMETA(DisplayName = "Falchion"),
        Greateaxe	UMETA(DisplayName = "Greateaxe"),
        Greatclub	UMETA(DisplayName = "Greatclub"),
        Halberd		UMETA(DisplayName = "Halberd"),
        Longbow		UMETA(DisplayName = "Longbow"),
        Shortbow	UMETA(DisplayName = "Shortbow"),
        Warglave	UMETA(DisplayName = "Warglave"),
        Shortsword	UMETA(DisplayName = "Shortsword"),
        Shield		UMETA(DisplayName = "Shield"),
        Unarmed     UMETA(DisplayName = "Unarmed")
    };


    UENUM(BlueprintType)
    enum class EShieldType : uint8
    {
        TowerShield UMETA(DisplayName = "TowerShield"),
        SmallShield UMETA(DisplayName = "SmallShield"),
        HeavyShield UMETA(DisplayName = "HeavyShield"),
        Buckler		UMETA(DisplayName = "Buckler"),
    };

    UENUM(BlueprintType)
    enum class EArmorType : uint8
    {
        Clothes			UMETA(DisplayName = "Clothes"),
        //Light		
        Padded			UMETA(DisplayName = "Padded"),
        Leather			UMETA(DisplayName = "Leather"),
        StuddedLeather	UMETA(DisplayName = "StuddedLeather"),
        Chainshirt		UMETA(DisplayName = "Chainshirt"),
        //Medium
        Hide			UMETA(DisplayName = "Hide"),
        Scalemail		UMETA(DisplayName = "Scalemail"),
        Chainmail		UMETA(DisplayName = "Chainmail"),
        Breastplate		UMETA(DisplayName = "Breastplate"),
        //Heavy
        Splintmail		UMETA(DisplayName = "Splintmail"),
        Bandedmail		UMETA(DisplayName = "Bandedmail"),
        Halfplate		UMETA(DisplayName = "Halfplate"),
        Fullplate		UMETA(DisplayName = "Fullplate"),
    };


    UENUM(BlueprintType)
    enum class EItemSlot : uint8
    {
        ChestSlot			UMETA(DisplayName = "Chest Slot"),   // Armor Slot 
        HeadSlot			UMETA(DisplayName = "Head Slot"),	 // Armor Slot
        LegSlot				UMETA(DisplayName = "Leg Slot"),     // Armor Slot
        FootSlot			UMETA(DisplayName = "Foot Slot"),    // Armor Slot
        HandsSlot			UMETA(DisplayName = "Hands Slot"),   // Armor Slot
        //BeltSlot			UMETA(DisplayName = "Belt Slot"),

        NeckSlot			UMETA(DisplayName = "Neck Slot"),   // M
        RightRingSlot		UMETA(DisplayName = "Right Ring Slot"),	 // M
        LeftRingSlot		UMETA(DisplayName = "Left Ring Slot"),   // M

        MainWeaponSlot		UMETA(DisplayName = "Right Weapon Slot"),   //Weapon
        OffMeaponSlot		UMETA(DisplayName = "Left Weapon Slot"),   //Weapon
        
        MainWeaponSlotSet2	UMETA(DisplayName = "Right Weapon Slot set 2"),   //Weapon
        OffWeaponSlotSet2	UMETA(DisplayName = "Left Weapon Slot set 2"),   //Weapon

        AmmoSlot            UMETA(DisplayName = "Ammo Slot"),    //Ammo
        
        Max					UMETA(Hidden)
    };


    UENUM(BlueprintType, meta = (Bitflags))
    enum class EItemFlags : uint8    //!< Флаги предметов.
    {
        NONE        = 0,
        Usable      = 1 UMETA(DisplayName = "Usable"),
        Consumable  = 2 UMETA(DisplayName = "Consumable"),
        Magical     = 4 UMETA(DisplayName = "Magical"),
        Cursed      = 8 UMETA(DisplayName = "Cursed"),
        Stolen      = 16 UMETA(DisplayName = "Stolen"),
        Unbreakable = 32 UMETA(DisplayName = "Unbreakable"),
        Unsellable  = 64 UMETA(DisplayName = "Unsellable"),
        Unstealable = 128 UMETA(DisplayName = "Unstealable")
    };
    ENUM_CLASS_FLAGS(EItemFlags)
// clang-format on
