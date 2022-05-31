#pragma once

#include "CoreMinimal.h"


DECLARE_DELEGATE_RetVal(bool, FLinkReq);
DECLARE_DELEGATE(FStateEvent);


struct StateInfoBase
{
    FStateEvent OnEnter;
    FStateEvent OnLeave;
};

template<typename StateInfo>
class FSM
{
public:
    struct State
    {
        friend class FSM;

        State(int inputId) {Id = inputId;}
        int Id;
        StateInfo Info;
        bool operator==(const State& other) const { return Id == other.Id; }

    private:
        using Link = TPair<int, FLinkReq>;
        TArray<Link> Links;
    };

    bool contains(int Id) const
    {
        if (_States.FindByKey(Id))
            return true;
        return false;
    }

    bool isInState(int Id) const
    {
        check(_CurrentState && "no state");
        return _CurrentState->Id == Id;
    }

    State *state(int StateId) 
    {
        check(contains(StateId) && "invalid state id");
        return &_States[StateId];
    }
    const State *state(int StateId) const
    {
        check(contains(StateId) && "invalid state id");
        return &_States[StateId];
    }

    void setCurrentState(int Id)
    {
        check(contains(Id) && "invalid state id");
        if (_CurrentState && _CurrentState->Id == Id)
            return;
        if (_CurrentState)
            _CurrentState->Info.OnLeave.ExecuteIfBound();
        _CurrentState = state(Id);
        _CurrentState->Info.OnEnter.ExecuteIfBound();
    }

    State *const currentState() { return _CurrentState; }

    const State *const currentState() const { return _CurrentState; }

    FSM &addState(int StateId)
    {
        if (!contains(StateId))
            _States.Emplace(State(StateId));
        return *this;
    }

    FSM &addLink(int FromState, int ToState, const FLinkReq& Req)
    {
        check(contains(FromState) && "invalid state id");
        auto from = state(FromState);
        from->Links.Emplace(MakeTuple<int&, const FLinkReq&>(ToState, Req));
        return *this;
    }

    const State* proccessNextState() const
    {
        if (_CurrentState)
        {
            for (const auto& link : _CurrentState->Links)
            {
                if (link.Value.Execute())
                    return state(link.Key);
            }
        }
        return nullptr;
    }

private:
    TArray<State> _States;
    State *_CurrentState = nullptr;
};


