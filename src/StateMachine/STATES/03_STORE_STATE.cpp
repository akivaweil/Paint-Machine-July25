//* ************************************************************************
//* ************************ STORE STATE *********************************
//* ************************************************************************
//! STORE state - Performs storing operations
//! This state handles the storage sequence for the paint machine

#include <Arduino.h>
#include "StateMachine.h"
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"
#include <FastAccelStepper.h>

//* ************************************************************************
//* ************************ STORE STATE VARIABLES ***********************
//* ************************************************************************
static bool storeStateInitialized = false;
static bool storeComplete = false;
static FastAccelStepper* storeZMotor = NULL;

//* ************************************************************************
//* ************************ STORE STATE FUNCTIONS ***********************
//* ************************************************************************

void executeStoreState() {
    //! ************************************************************************
    //! EXECUTE STORE STATE - STORAGE OPERATION
    //! ************************************************************************
    
    // Initialize store state on first entry
    if (!storeStateInitialized) {
        Serial.println("=== ENTERING STORE STATE ===");
        Serial.println("Starting storage operation...");
        storeStateInitialized = true;
        storeComplete = false;
        
        //! ************************************************************************
        //! STEP 1: SET STORING SPEED
        //! ************************************************************************
        if (storeZMotor) {
            storeZMotor->setSpeedInHz(Z_MAX_SPEED);
            Serial.println("Z Motor speed set to: " + String(Z_MAX_SPEED) + " Hz");
        }
    }
    
    //! ************************************************************************
    //! STEP 2: CHECK IF STORING IS COMPLETE
    //! ************************************************************************
    if (storeComplete) {
        Serial.println("Storage operation complete - transitioning to IDLE");
        setState(IDLE_STATE);
        return;
    }
    
    //! ************************************************************************
    //! STEP 3: PERFORM STORING SEQUENCE
    //! ************************************************************************
    if (storeZMotor) {
        //! ************************************************************************
        //! STEP 3A: MOVE TO STORAGE POSITION
        //! ************************************************************************
        if (!storeZMotor->isRunning()) {
            // Move to storage position (example: 10 inches from home)
            int storagePosition = 10 * STEPS_PER_INCH; // 10 inches
            storeZMotor->moveTo(storagePosition);
            Serial.println("Moving to storage position: " + String(storagePosition) + " steps");
        }
        
        //! ************************************************************************
        //! STEP 3B: CHECK IF MOVEMENT COMPLETE
        //! ************************************************************************
        if (!storeZMotor->isRunning()) {
            Serial.println("Reached storage position");
            
            //! ************************************************************************
            //! STEP 3C: PERFORM STORAGE ACTIONS
            //! ************************************************************************
            // Add specific storage logic here
            // For example: release gripper, servo movements, etc.
            
            Serial.println("Storage actions completed");
            storeComplete = true;
        }
    } else {
        Serial.println("ERROR: Z motor not available");
        setState(IDLE_STATE);
    }
}

void resetStoreState() {
    //! ************************************************************************
    //! RESET STORE STATE FLAGS
    //! ************************************************************************
    storeStateInitialized = false;
    storeComplete = false;
}

void setStoreReferences(FastAccelStepper* motor) {
    //! ************************************************************************
    //! SET REFERENCES TO MOTOR OBJECTS
    //! ************************************************************************
    storeZMotor = motor;
} 