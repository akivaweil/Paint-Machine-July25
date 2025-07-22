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
        
        // Reset previous state when transitioning away from it
        switch (previousState) {
            case IDLE_STATE:
                resetIdleState();
                break;
            case HOME_STATE:
                resetHomeState();
                break;
            case PICK_STATE:
                resetPickState();
                break;
            case PLACE_STATE:
                resetPlaceState();
                break;
            case TEST_STATE:
                resetTestState();
                break;
            case CELL_SEQUENCE_STATE:
                resetCellSequenceState();
                break;
            case CELL_TEST_STATE:
                resetCellTestState();
                break;
            default:
                break;
        }
        
        stateChanged = false;
    }
    
    // Execute current state
    switch (currentState) {
        case IDLE_STATE:
            executeIdleState();
            break;
        case HOME_STATE:
            executeHomeState();
            break;
        case PICK_STATE:
            executePickState();
            break;
        case PLACE_STATE:
            executePlaceState();
            break;
        case TEST_STATE:
            executeTestState();
            break;
        case CELL_SEQUENCE_STATE:
            executeCellSequenceState();
            break;
        case CELL_TEST_STATE:
            executeCellTestState();
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
        case HOME_STATE:
            return "HOME";
        case PICK_STATE:
            return "PICK";
        case PLACE_STATE:
            return "PLACE";
        case TEST_STATE:
            return "TEST";
        case CELL_SEQUENCE_STATE:
            return "CELL_SEQUENCE";
        case CELL_TEST_STATE:
            return "CELL_TEST";
        default:
            return "UNKNOWN";
    }
} 