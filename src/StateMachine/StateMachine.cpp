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
            case RETRIEVE_STATE:
                resetRetrieveState();
                break;
            case STORE_STATE:
                resetStoreState();
                break;
            case RETRIEVE_2_STATE:
                resetRetrieve2State();
                break;
            case STORE_2_STATE:
                resetStore2State();
                break;
            case TEST_STATE:
                resetTestState();
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
        case RETRIEVE_STATE:
            executeRetrieveState();
            break;
        case STORE_STATE:
            executeStoreState();
            break;
        case RETRIEVE_2_STATE:
            executeRetrieve2State();
            break;
        case STORE_2_STATE:
            executeStore2State();
            break;
        case TEST_STATE:
            executeTestState();
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
        case RETRIEVE_STATE:
            return "RETRIEVE";
        case STORE_STATE:
            return "STORE";
        case RETRIEVE_2_STATE:
            return "RETRIEVE_2";
        case STORE_2_STATE:
            return "STORE_2";
        case TEST_STATE:
            return "TEST";
        default:
            return "UNKNOWN";
    }
} 