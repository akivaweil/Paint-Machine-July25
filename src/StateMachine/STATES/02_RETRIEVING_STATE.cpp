//* ************************************************************************
//* ************************ RETRIEVING STATE *****************************
//* ************************************************************************
//! RETRIEVING state - Performs retrieving operations
//! This state handles the retrieval sequence for the paint machine

#include <Arduino.h>
#include "StateMachine.h"
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"
#include <FastAccelStepper.h>

//* ************************************************************************
//* ************************ RETRIEVING STATE VARIABLES *******************
//* ************************************************************************
static bool retrievingStateInitialized = false;
static bool retrievingComplete = false;
static FastAccelStepper* retrievingZMotor = NULL;

//* ************************************************************************
//* ************************ RETRIEVING STATE FUNCTIONS *******************
//* ************************************************************************

void executeRetrievingState() {
    //! ************************************************************************
    //! EXECUTE RETRIEVING STATE - RETRIEVAL OPERATION
    //! ************************************************************************
    
    // Initialize retrieving state on first entry
    if (!retrievingStateInitialized) {
        Serial.println("=== ENTERING RETRIEVING STATE ===");
        Serial.println("Starting retrieval operation...");
        retrievingStateInitialized = true;
        retrievingComplete = false;
        
        //! ************************************************************************
        //! STEP 1: SET RETRIEVING SPEED
        //! ************************************************************************
        if (retrievingZMotor) {
            retrievingZMotor->setSpeedInHz(Z_MAX_SPEED);
            Serial.println("Z Motor speed set to: " + String(Z_MAX_SPEED) + " Hz");
        }
    }
    
    //! ************************************************************************
    //! STEP 2: CHECK IF RETRIEVING IS COMPLETE
    //! ************************************************************************
    if (retrievingComplete) {
        Serial.println("Retrieval operation complete - transitioning to IDLE");
        setState(IDLE_STATE);
        return;
    }
    
    //! ************************************************************************
    //! STEP 3: PERFORM RETRIEVING SEQUENCE
    //! ************************************************************************
    if (retrievingZMotor) {
        //! ************************************************************************
        //! STEP 3A: MOVE TO RETRIEVAL POSITION
        //! ************************************************************************
        if (!retrievingZMotor->isRunning()) {
            // Move to retrieval position (example: 5 inches from home)
            int retrievalPosition = 5 * STEPS_PER_INCH; // 5 inches
            retrievingZMotor->moveTo(retrievalPosition);
            Serial.println("Moving to retrieval position: " + String(retrievalPosition) + " steps");
        }
        
        //! ************************************************************************
        //! STEP 3B: CHECK IF MOVEMENT COMPLETE
        //! ************************************************************************
        if (!retrievingZMotor->isRunning()) {
            Serial.println("Reached retrieval position");
            
            //! ************************************************************************
            //! STEP 3C: PERFORM RETRIEVAL ACTIONS
            //! ************************************************************************
            // Add specific retrieval logic here
            // For example: activate gripper, servo movements, etc.
            
            Serial.println("Retrieval actions completed");
            retrievingComplete = true;
        }
    } else {
        Serial.println("ERROR: Z motor not available");
        setState(IDLE_STATE);
    }
}

void resetRetrievingState() {
    //! ************************************************************************
    //! RESET RETRIEVING STATE FLAGS
    //! ************************************************************************
    retrievingStateInitialized = false;
    retrievingComplete = false;
}

void setRetrievingReferences(FastAccelStepper* motor) {
    //! ************************************************************************
    //! SET REFERENCES TO MOTOR OBJECTS
    //! ************************************************************************
    retrievingZMotor = motor;
} 