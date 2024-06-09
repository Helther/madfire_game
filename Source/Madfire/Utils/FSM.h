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
        if (_States.Contains(Id))
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
            _States.Emplace(StateId, State(StateId));
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

    const State &operator[](int StateId) const
    {
        check(contains(StateId) && "invalid state id");
        return _States[StateId];
    }

    State &operator[](int StateId) 
    {
        check(contains(StateId) && "invalid state id");
        return _States[StateId];
    }

    FSM &add(int StateId)
    {
        if (!contains(StateId))
            _States.Emplace(StateId, State(StateId));
        return *this;
    }

    FSM &link(int FromState, int ToState, const FLinkReq& Req)
    {
        check(contains(FromState) && "invalid state id");
        auto from = state(FromState);
        from->Links.Emplace(MakeTuple<int&, const FLinkReq&>(ToState, Req));
        return *this;
    }

private:
    TMap<int32, State> _States;
    State *_CurrentState = nullptr;
};

template<typename StateInfo>
class FSMv2
{
public:
    struct State : public StateInfo
    {
        friend class FSMv2;

        State(int inputId) {Id = inputId;}
        int Id;
        bool operator==(const State& other) const { return Id == other.Id; }

        //FStateEvent OnEnter;
        //FStateEvent OnLeave;

    private:
        using Link = TPair<int, FLinkReq>;
        TArray<Link> Links;
    };

    bool contains(int Id) const
    {
        if (_States.Contains(Id))
            return true;
        return false;
    }

    bool isInState(int Id) const
    {
        check(_CurrentState && "no state");
        return _CurrentState->Id == Id;
    }

    bool is(int Id) const
    {
        check(_CurrentState && "no state");
        return _CurrentState->Id == Id;
    }

    void setCurrentState(int Id)
    {
        check(contains(Id) && "invalid state id");
        if (_CurrentState)
            (*_CurrentState).OnLeave.ExecuteIfBound();
        _CurrentState = &_States[Id];
        (*_CurrentState).OnEnter.ExecuteIfBound();
    }

    State *const currentState() 
    {
        return _CurrentState; 
    }

    const State *const currentState() const 
    {
        return _CurrentState; 
    }

    const State* update() const
    {
        if (_CurrentState)
        {
            for (const auto& link : _CurrentState->Links)
            {
                check(link.Value.IsBound() && "Condition is invalid");
                if (link.Value.Execute())
                    return &_States[link.Key];
            }
        }
        return nullptr;
    }

    const State &operator[](int StateId) const
    {
        check(contains(StateId) && "invalid state id");
        return _States[StateId];
    }

    State &operator[](int StateId) 
    {
        check(contains(StateId) && "invalid state id");
        return _States[StateId];
    }

    FSMv2 &add(int StateId)
    {
        if (!contains(StateId))
            _States.Emplace(StateId, State(StateId));
        return *this;
    }

    FSMv2 &link(int FromState, int ToState, const FLinkReq& Req)
    {
        check(contains(FromState) && "invalid state id");
        auto &from = _States[FromState];
        from.Links.Emplace(MakeTuple<int&, const FLinkReq&>(ToState, Req));
        return *this;
    }

    explicit operator bool() const
    {
        return _CurrentState != nullptr;
    }

    void clear() 
    {
        _States.Empty();
    }

private:
    TMap<int32, State> _States;
    State *_CurrentState = nullptr;
};
