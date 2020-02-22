// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

DECLARE_DELEGATE_RetVal(bool, FLinkReq);

struct Attack
{
    class UAnimMontage *AttackMontage;
    FName AnimSectionName;
    float DamageMod;
    float PowerMod;
    float StaminaCost;
    bool IsBlockablel;
    bool IsParriable;
    bool IsInterruptable;

    //some special delegate shit
};


template<typename StateInfo>
class ComboFSM
{
    struct State
    {
        FName Id;
        StateInfo info;

        using Link = TPair<const State &, FLinkReq>;
        TArray<Link> Links;
    };

public:
    ComboFSM(){}
    ~ComboFSM(){}

    bool contain(FName StateName)
    {
        if(_States.Contains(StateName))
                return true;
        return false;
    }

    State &state(FName StateName)
    {
        return _States.Find(StateName);
    }

    void setCurrentState(FName StateName)
    {
        if(_CurrentState->Id != StateName)
            _CurrentState = state(StateName);
    }

    State &currentState()
    {
        return _CurrentState;
    }

    ComboFSM &addState(FName StateName)
    {
        if(!contain(StateName))
            _States.Push(state(StateName));
        return *this;
    }
    ComboFSM &addLink(FName FromState, FName ToState, FLinkReq)
    {
        auto &from = state(FromState);
        const auto &to = state(ToState);
        const auto &NewLink = MakeTuple<to, FLinkReq>;
        from.Links.Push(NewLink);
        return *this;
    }

private:
    TArray<State> _States;
    State *_CurrentState = nullptr;
};
