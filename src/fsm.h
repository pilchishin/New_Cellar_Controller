#ifndef FSM_H
#define FSM_H

enum class State {
    IDLE,
    MONITORING,
    CLIMATE_CONTROL,
    OZONE_TREATMENT,
    EMERGENCY,
    CALIBRATION,
    MENU
};

class FSM {
public:
    FSM();
    void init();
    void update();
    
    State getCurrentState();
    void setState(State newState);
    
    void handleInput();
    void processState();
    
private:
    State currentState;
    unsigned long stateStartTime;
    unsigned long stateTimeout;
};

#endif // FSM_H