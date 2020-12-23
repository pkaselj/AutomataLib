#include"Automata.hpp"

#include<stdexcept>

//================================EVENT

Event::Event(const std::string& event_name)
{
    if(event_name == "")
    {
        exit(-1);
    }

    name = event_name;
}

std::string Event::getName() const
{
    return name;
}

//================================STATE

State::State(const std::string& state_name, state_action _action)
{
    action = nullptr;
    name = "";

    if(state_name == "")
    {
        exit(-1);
    }

    name = state_name;
    action = _action;
}

std::string State::getName() const
{
    return name;
}

void State::execute(void* data, void* arguments) const
{
    if(action != nullptr)
        action(data, arguments);
}

//================================STATE_EVENT_PAIR

State_Event_Pair::State_Event_Pair(const State* _state, const Event* _event)
{
    state = _state;
    event = _event;

    if(state == nullptr)
    {
        exit(-1);
    }

    if(event == nullptr)
    {
        exit(-1);
    }
}

std::string State_Event_Pair::getUniqueIdentifier() const
{
    return state->getName() + event->getName();
}

State_Event_Pair* operator+ (State& _state, Event& _event)
{
    State_Event_Pair* state_event_pair = new State_Event_Pair(&_state, &_event);
    return state_event_pair;
}

//================================TRANSITION

Transition::Transition(State_Event_Pair* _p_state_event_pair, State* _p_next_state)
{
    p_next_state = _p_next_state;

    if(p_next_state == nullptr)
    {
        exit(-1);
    }

    p_state_event_pair = _p_state_event_pair;

    if(p_state_event_pair == nullptr)
    {
        exit(-1);
    }
}

State* Transition::getNextState() const
{
    return p_next_state;
}

State_Event_Pair* Transition::getStateEventPair() const
{
    return p_state_event_pair;
}

Transition* operator> (State_Event_Pair* p_state_event_pair, State& next_state)
{
    Transition* current_Transition = new Transition(p_state_event_pair, &next_state);
    return current_Transition;
}

//================================TABLE

const State* Table::next_state(const State* current_state, const Event* event) const
{
    const State* next_state_pointer = nullptr;
    try
    {
        State_Event_Pair current_state_event(current_state, event);
        next_state_pointer = transition_table.at( current_state_event.getUniqueIdentifier() );
    }
    catch(std::out_of_range& exception)
    {
        // Error
        // Undefined state event transition
        next_state_pointer = current_state;
    }

    return next_state_pointer;

}


Table& operator<<(Table& table, Transition* p_transition)
{
    table.transition_table.insert({
                                    p_transition->getStateEventPair()->getUniqueIdentifier(),
                                    p_transition->getNextState()
                                  });

    delete p_transition->getStateEventPair(); // Dynamically allocated when using operator+ on State and Event
    delete p_transition; // Dynamically allocated when using operator> on State_Event_Pair

    return table;
}

//================================AUTOMATA

Automata::Automata(const State& _p_current_state,
                   const State& _p_starting_state,
                   const State& _p_exit_state,
                   const Table& _p_transition_table,
                   void* _data)
{
    p_current_state = &_p_current_state;
    if(p_current_state == nullptr)
    {
        exit(-1);
    }

    p_starting_state = &_p_starting_state;
    if(p_starting_state == nullptr)
    {
        exit(-1);
    }

    p_exit_state = &_p_exit_state;
    if(p_exit_state == nullptr)
    {
        exit(-1);
    }

    p_transition_table = &_p_transition_table;
    if(p_transition_table == nullptr)
    {
        exit(-1);
    }

    data = _data;
}

void Automata::Reset()
{
    p_current_state = p_starting_state;
}

void Automata::Advance(Event& event, void* arguments)
{
    p_current_state = p_transition_table->next_state(p_current_state, &event);
    // check if staying in the same state TODO
    p_current_state->execute(data, arguments);
}