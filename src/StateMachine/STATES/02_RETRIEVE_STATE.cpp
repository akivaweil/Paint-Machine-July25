//* ************************************************************************
//* ************************ RETRIEVE STATE ******************************
//* ************************************************************************
//! RETRIEVE state - Performs retrieving operations
//! This state handles the retrieval sequence for the paint machine

#include <Arduino.h>
#include "StateMachine.h"
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"
#include <FastAccelStepper.h>

//* ************************************************************************
//* ************************ RETRIEVE STATE VARIABLES ********************
//* ************************************************************************
static bool retrieveStateInitialized = false;
static bool retrieveComplete = false;
static FastAccelStepper* retrieveZMotor = NULL;

//* ************************************************************************
//* ************************ RETRIEVE STATE FUNCTIONS ********************
//* ************************************************************************

void executeRetrieveState() {
    //! ************************************************************************
    //! EXECUTE RETRIEVE STATE - RETRIEVAL OPERATION
    //! ************************************************************************
    
    // Initialize retrieve state on first entry
    if (!retrieveStateInitialized) {
        Serial.println("=== ENTERING RETRIEVE STATE ===");
        Serial.println("Starting retrieval operation...");
        retrieveStateInitialized = true;
        retrieveComplete = false;
        
        //! ************************************************************************
        //! STEP 1: SET RETRIEVING SPEED
        //! ************************************************************************
        if (retrieveZMotor) {
            retrieveZMotor->setSpeedInHz(Z_MAX_SPEED);
            Serial.println("Z Motor speed set to: " + String(Z_MAX_SPEED) + " Hz");
        }
    }
    
    //! ************************************************************************
    //! STEP 2: CHECK IF RETRIEVING IS COMPLETE
    //! ************************************************************************
    if (retrieveComplete) {
        Serial.println("Retrieval operation complete - transitioning to IDLE");
        setState(IDLE_STATE);
        return;
    }
    
    //! ************************************************************************
    //! STEP 3: PERFORM RETRIEVING SEQUENCE
    //! ************************************************************************
    if (retrieveZMotor) {
        //! ************************************************************************
        //! STEP 3A: MOVE TO RETRIEVAL POSITION
        //! ************************************************************************
        if (!retrieveZMotor->isRunning()) {
            // Move to retrieval position (example: 5 inches from home)
            int retrievalPosition = 5 * STEPS_PER_INCH; // 5 inches
            retrieveZMotor->moveTo(retrievalPosition);
            Serial.println("Moving to retrieval position: " + String(retrievalPosition) + " steps");
        }
        
        //! ************************************************************************
        //! STEP 3B: CHECK IF MOVEMENT COMPLETE
        //! ************************************************************************
        if (!retrieveZMotor->isRunning()) {
            Serial.println("Reached retrieval position");
            
            //! ************************************************************************
            //! STEP 3C: PERFORM RETRIEVAL ACTIONS
            //! ************************************************************************
            // Add specific retrieval logic here
            // For example: activate gripper, servo movements, etc.
            
            Serial.println("Retrieval actions completed");
            retrieveComplete = true;
        }
    } else {
        Serial.println("ERROR: Z motor not available");
        setState(IDLE_STATE);
    }
}

void resetRetrieveState() {
    //! ************************************************************************
    //! RESET RETRIEVE STATE FLAGS
    //! ************************************************************************
    retrieveStateInitialized = false;
    retrieveComplete = false;
}

void setRetrieveReferences(FastAccelStepper* motor) {
    //! ************************************************************************
    //! SET REFERENCES TO MOTOR OBJECTS
    //! ************************************************************************
    retrieveZMotor = motor;
} 