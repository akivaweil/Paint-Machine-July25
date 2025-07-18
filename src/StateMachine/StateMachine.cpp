//* ************************************************************************
//* ************************ STATE MACHINE ********************************
//* ************************************************************************
//! Main state machine controller for Paint Machine
//! Manages state transitions and state execution

#include <Arduino.h>
#include "StateMachine.h"

//* ************************************************************************
//* ************************ STATE VARIABLES *******************************
//* ************************************************************************
StateMachineState currentState = IDLE_STATE;
StateMachineState previousState = IDLE_STATE;
bool stateChanged = false;

//* ************************************************************************
//* ************************ STATE MACHINE FUNCTIONS **********************
//* ************************************************************************

void initStateMachine() {
    //! ************************************************************************
    //! INITIALIZE STATE MACHINE
    //! ************************************************************************
    currentState = IDLE_STATE;
    previousState = IDLE_STATE;
    stateChanged = true;
    
    Serial.println("State Machine initialized - Starting in IDLE state");
}

void updateStateMachine() {
    //! ************************************************************************
    //! UPDATE STATE MACHINE - CALLED IN MAIN LOOP
    //! ************************************************************************
    
    // Check if state has changed
    if (stateChanged) {
        Serial.println("=== STATE CHANGE: " + getStateName(previousState) + " -> " + getStateName(currentState) + " ===");
        stateChanged = false;
    }
    
    // Execute current state
    switch (currentState) {
        case IDLE_STATE:
            executeIdleState();
            break;
        case HOMING_STATE:
            executeHomingState();
            break;
        case RETRIEVING_STATE:
            executeRetrievingState();
            break;
        case STORING_STATE:
            executeStoringState();
            break;
        default:
            Serial.println("ERROR: Unknown state encountered");
            setState(IDLE_STATE);
            break;
    }
}

void setState(StateMachineState newState) {
    //! ************************************************************************
    //! CHANGE STATE WITH VALIDATION
    //! ************************************************************************
    if (newState != currentState) {
        previousState = currentState;
        currentState = newState;
        stateChanged = true;
    }
}

StateMachineState getCurrentState() {
    return currentState;
}

StateMachineState getPreviousState() {
    return previousState;
}

bool isStateChanged() {
    return stateChanged;
}

String getStateName(StateMachineState state) {
    //! ************************************************************************
    //! CONVERT STATE ENUM TO STRING FOR DEBUGGING
    //! ************************************************************************
    switch (state) {
        case IDLE_STATE:
            return "IDLE";
        case HOMING_STATE:
            return "HOMING";
        case RETRIEVING_STATE:
            return "RETRIEVING";
        case STORING_STATE:
            return "STORING";
        default:
            return "UNKNOWN";
    }
} 