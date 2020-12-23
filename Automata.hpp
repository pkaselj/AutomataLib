#ifndef AUTOMATA_HPP
#define AUTOMATA_HPP

#include<string>
#include<unordered_map>

#define ARG_COUNT(_1, _2, COUNT, ...) COUNT
#define COUNT(...) ARG_COUNT(__VA_ARGS__, 2, 1)

#define CONCAT(A, B) A ## B
#define SELECT(MACRO, NUM) CONCAT(MACRO ##_ , NUM)

#define VARIADIC_SELECT(MACRO, ...) SELECT(MACRO, COUNT(__VA_ARGS__))(__VA_ARGS__)

#define STATE(...) VARIADIC_SELECT(STATE, __VA_ARGS__)
#define STATE_1(NAME) State NAME(#NAME);
#define STATE_2(NAME, ACTION) State NAME(#NAME, ACTION);

#define EVENT(NAME) Event NAME(#NAME);

class Event
{
    private:
    std::string name;

    public:
    Event(const std::string& event_name);
    ~Event() = default;
    Event(Event&) = delete;
    Event(Event&&) = delete;
    void operator= (Event&) = delete;

    std::string getName() const;
};

typedef bool (*state_action) (void* data, void* arguments);

class State
{
    private:
    state_action action;
    std::string name;

    public:
    State(const std::string& state_name, state_action _action = nullptr);
    ~State() = default;
    State(State&) = delete;
    State(State&&) = delete;
    void operator= (State&) = delete;

    std::string getName() const;
    void execute(void* data, void* arguments) const;
};

class State_Event_Pair
{
    private:
    const State* state = nullptr;
    const Event* event = nullptr;

    public:
    State_Event_Pair(const State* _state, const Event* _event);
    State_Event_Pair() = delete;
    State_Event_Pair(State_Event_Pair&) = default;
    State_Event_Pair(State_Event_Pair&&) = default;
    ~State_Event_Pair() = default;

    std::string getUniqueIdentifier() const;
};

State_Event_Pair* operator+ (State& _state, Event& _event);

class Transition
{
    private:
    State_Event_Pair* p_state_event_pair = nullptr;
    State* p_next_state = nullptr;

    public:
    Transition(State_Event_Pair* _p_state_event_pair, State* _p_next_state);
    Transition() = default;
    ~Transition() = default;
    Transition(Transition&) = default;
    Transition(Transition&&) = default;

    State* getNextState() const;
    State_Event_Pair* getStateEventPair() const;

    
};

Transition* operator> (State_Event_Pair* p_state_event_pair, State& next_state);

class Table
{
    private:

    std::unordered_map<std::string, State*> transition_table;

    public:
    Table() = default;
    ~Table() = default;
    Table(Table&) = default;
    Table(Table&&) = default;

    const State* next_state(const State* current_state, const Event* event) const;

    friend Table& operator<<(Table& table, Transition* p_transition);

};

class Automata
{
    private:
    const State* p_current_state = nullptr;
    const State* p_starting_state = nullptr;
    const State* p_exit_state = nullptr;

    const Table* p_transition_table = nullptr;

    void* data = nullptr;

    public:
    Automata(const State& _p_current_state,
             const State& _p_starting_state,
             const State& _p_exit_state,
             const Table& _p_transition_table,
             void* _data = nullptr);
    ~Automata() = default;
    Automata(Automata&) = default;
    Automata(Automata&&) = default;

    void Reset();
    void Advance(Event& event, void* arguments = nullptr);
};
#endif