#include"Automata.hpp"

#include"/home/pi/Shared/KernelLib/0.0.0/Kernel.hpp"
#include"/home/pi/Shared/LoggerLib/0.0.0/NulLogger.hpp"

#include<stdexcept>

//================================EVENT

Event::Event(const std::string& event_name)
{
    if(event_name == "")
    {
        Kernel::Fatal_Error("Event name cannot be empty");
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
        Kernel::Fatal_Error("State name cannot be empty");

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
    else
        Kernel::Warning("Trying to execute nonexistent action assigned to " + name + " state!");
}

//================================STATE_EVENT_PAIR

State_Event_Pair::State_Event_Pair(const State* _state, const Event* _event)
{
    state = _state;
    event = _event;

    if(state == nullptr)
    {
        Kernel::Fatal_Error("State_Event_Pair - state error! nullptr!");
    }

    if(event == nullptr)
    {
        Kernel::Fatal_Error("State_Event_Pair - event error! nullptr!");
    }
}

std::string State_Event_Pair::getUniqueIdentifier() const
{
    return state->getName() + event->getName();
}

State_Event_Pair* operator+ (State& _state, Event& _event)
{
    State_Event_Pair* state_event_pair = new State_Event_Pair(&_state, &_event);
    if(state_event_pair == nullptr)
        Kernel::Fatal_Error("Could not create a new state event pair object for the following arguments: (" + _state.getName() + ", " + _event.getName() + ")");
    return state_event_pair;
}

//================================TRANSITION

Transition::Transition(State_Event_Pair* _p_state_event_pair, State* _p_next_state)
{
    p_next_state = _p_next_state;

    if(p_next_state == nullptr)
    {
        Kernel::Fatal_Error("Transition - next state error! nullptr!");
    }

    p_state_event_pair = _p_state_event_pair;

    if(p_state_event_pair == nullptr)
    {
        Kernel::Fatal_Error("Transition - State_Event_Pair error! nullptr!");
    }
}

Transition::~Transition()
{
    delete p_state_event_pair;
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
        Kernel::Warning("No known transition in the transition table for: " + current_state->getName() + " and " + event->getName());
    }

    return next_state_pointer;

}


Table& operator<<(Table& table, Transition* p_transition)
{
    table.transition_table.insert({
                                    p_transition->getStateEventPair()->getUniqueIdentifier(),
                                    p_transition->getNextState()
                                  });

    // Write a destructor for Transition
    //delete p_transition->getStateEventPair(); // Dynamically allocated when using operator+ on State and Event
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
    LoadTable(_p_current_state, _p_starting_state, _p_exit_state, _p_transition_table, _data);
    setLogger(nullptr);
}

Automata::Automata(const State& _p_current_state,
                   const State& _p_starting_state,
                   const State& _p_exit_state,
                   const Table& _p_transition_table)
{
    LoadTable(_p_current_state, _p_starting_state, _p_exit_state, _p_transition_table);
    setLogger(nullptr);
}

void Automata::SetData(void* _data)
{
    data = _data;
    if(data == nullptr)
    {
        Kernel::Warning("Automaton not working on any data");
    }
}

void Automata::LoadTable(const State& _p_current_state,
                         const State& _p_starting_state,
                         const State& _p_exit_state,
                         const Table& _p_transition_table)
{
    p_current_state = &_p_current_state;
    if(p_current_state == nullptr)
    {
        Kernel::Fatal_Error("Automaton current state cannot be null");
    }

    p_starting_state = &_p_starting_state;
    if(p_starting_state == nullptr)
    {
        Kernel::Fatal_Error("Automaton starting state cannot be null");
    }

    p_exit_state = &_p_exit_state;
    if(p_exit_state == nullptr)
    {
        Kernel::Fatal_Error("Automaton exit state cannot be null");
    }

    p_transition_table = &_p_transition_table;
    if(p_transition_table == nullptr)
    {
        Kernel::Fatal_Error("Automaton transition table cannot be null");
    }
}

void Automata::setLogger(ILogger* _p_logger)
{
    p_logger = _p_logger;
    if(p_logger == nullptr)
        p_logger = NulLogger::getInstance();
}

void Automata::LoadTable(const State& _p_current_state,
                         const State& _p_starting_state,
                         const State& _p_exit_state,
                         const Table& _p_transition_table,
                         void* _data)
{
    LoadTable(_p_current_state, _p_starting_state, _p_exit_state, _p_transition_table);
    SetData(_data);
}

void Automata::Reset()
{
    p_current_state = p_starting_state;
}

void Automata::Advance(Event& event, void* arguments)
{
    const State* p_previous_state = p_current_state;

    p_current_state = p_transition_table->next_state(p_current_state, &event);

    *p_logger << p_previous_state->getName() + " + " + p_current_state->getName() + " -> " + event.getName();
    // check if staying in the same state TODO
    p_current_state->execute(data, arguments);
}
