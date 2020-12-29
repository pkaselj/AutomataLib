#ifndef AUTOMATA_HPP
#define AUTOMATA_HPP

#include"/home/pi/Shared/LoggerLib/0.0.0/ILogger.hpp"

#include<string>
#include<unordered_map>
#include<functional>

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
    Event() = delete;
    ~Event() = default;
    Event(const Event&) = default;
    Event(Event&&) = default;
    Event& operator= (const Event&) = default;

    friend bool operator== (const Event& event1, const Event& event2);

    std::string getName() const;
};

/// Event used by unstable states to advance automaton
static EVENT(NEXT_STATE_EVENT)
/// Null event - operation -> stays in current state but re-does the associated state action
static EVENT(NULL_EVENT_OP)
/// Null veent - no-operation -> stays in current state and does not re-do the associated state action
static EVENT(NULL_EVENT_NOP)
/// Default error event
static EVENT(ERROR_EVENT)

typedef bool (*state_action) (void* data, void* arguments);

class State
{
    private:
    //state_action action;
    state_action action;
    std::string name;

    public:
    State(const std::string& state_name, state_action _action = nullptr);
    State() = delete;
    //State(const std::string& state_name, std::function<bool(void*, void*)> _action = nullptr);
    ~State() = default;
    State(const State&) = default;
    State(State&&) = default;
    State& operator= (const State&) = default;

    friend bool operator== (const State& state1, const State& state2);

    std::string getName() const;
    bool execute(void* data, void* arguments) const;
};


/// Null (default starting) state
static STATE(NULL_STATE)

class State_Event_Pair
{
    private:
    const State state;
    const Event event;

    public:
    State_Event_Pair(const State _state, const Event _event);
    State_Event_Pair() = delete;
    State_Event_Pair(State_Event_Pair&) = default;
    State_Event_Pair(State_Event_Pair&&) = default;
    void operator= (State_Event_Pair&) = delete;
    ~State_Event_Pair() = default;

    std::string getUniqueIdentifier() const;
};

State_Event_Pair* operator+ (State& _state, Event& _event);

class Transition
{
    private:
    State_Event_Pair* p_state_event_pair = nullptr;
    State next_state;

    public:
    Transition(State_Event_Pair* _p_state_event_pair, State _next_state);
    Transition() = default;
    ~Transition();
    Transition(Transition&) = default;
    Transition(Transition&&) = default;

    const State& getNextState() const;
    State_Event_Pair* getStateEventPair() const;

    
};

Transition* operator> (State_Event_Pair* p_state_event_pair, State& next_state);

class Table
{
    private:

    std::unordered_map<std::string, State> transition_table;



    public:
    Table() = default;
    ~Table() = default;
    Table(Table&) = default;
    Table(Table&&) = default;

    State startingState = NULL_STATE;
    State exitState = NULL_STATE;

    /*void setStartingState(const State& _startingState);
    void setExitState(const State& _exitState);*/

    const State* next_state(const State* current_state, const Event& event) const;

    friend Table& operator<<(Table& table, Transition* p_transition);

    void DumpTable(const std::string& dump_file_name);

};

class Automata
{
    private:

    const State* p_current_state = nullptr;
    const Table* p_transition_table = nullptr;

    ILogger* p_logger = nullptr;

    void* data = nullptr;

    public:
    Automata(const Table& _p_transition_table,
             void* _data = nullptr);

    Automata() = default;
    ~Automata() = default;
    Automata(Automata&) = default;
    Automata(Automata&&) = default;

    // Temporary TODO fix
    void setLogger(ILogger* _p_logger);

    void LoadTable(const Table& _p_transition_table, void* data = nullptr);

    void SetData(void* _data);
    void Reset();
    void Advance(Event& event, void* arguments = nullptr);
};
#endif