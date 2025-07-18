//* ************************************************************************
//* ************************ STORING STATE ********************************
//* ************************************************************************
//! STORING state - Performs storing operations
//! This state handles the storage sequence for the paint machine

#include <Arduino.h>
#include "StateMachine.h"
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"
#include <FastAccelStepper.h>

//* ************************************************************************
//* ************************ STORING STATE VARIABLES **********************
//* ************************************************************************
static bool storingStateInitialized = false;
static bool storingComplete = false;
static FastAccelStepper* storingZMotor = NULL;

//* ************************************************************************
//* ************************ STORING STATE FUNCTIONS **********************
//* ************************************************************************

void executeStoringState() {
    //! ************************************************************************
    //! EXECUTE STORING STATE - STORAGE OPERATION
    //! ************************************************************************
    
    // Initialize storing state on first entry
    if (!storingStateInitialized) {
        Serial.println("=== ENTERING STORING STATE ===");
        Serial.println("Starting storage operation...");
        storingStateInitialized = true;
        storingComplete = false;
        
        //! ************************************************************************
        //! STEP 1: SET STORING SPEED
        //! ************************************************************************
        if (storingZMotor) {
            storingZMotor->setSpeedInHz(Z_MAX_SPEED);
            Serial.println("Z Motor speed set to: " + String(Z_MAX_SPEED) + " Hz");
        }
    }
    
    //! ************************************************************************
    //! STEP 2: CHECK IF STORING IS COMPLETE
    //! ************************************************************************
    if (storingComplete) {
        Serial.println("Storage operation complete - transitioning to IDLE");
        setState(IDLE_STATE);
        return;
    }
    
    //! ************************************************************************
    //! STEP 3: PERFORM STORING SEQUENCE
    //! ************************************************************************
    if (storingZMotor) {
        //! ************************************************************************
        //! STEP 3A: MOVE TO STORAGE POSITION
        //! ************************************************************************
        if (!storingZMotor->isRunning()) {
            // Move to storage position (example: 10 inches from home)
            int storagePosition = 10 * STEPS_PER_INCH; // 10 inches
            storingZMotor->moveTo(storagePosition);
            Serial.println("Moving to storage position: " + String(storagePosition) + " steps");
        }
        
        //! ************************************************************************
        //! STEP 3B: CHECK IF MOVEMENT COMPLETE
        //! ************************************************************************
        if (!storingZMotor->isRunning()) {
            Serial.println("Reached storage position");
            
            //! ************************************************************************
            //! STEP 3C: PERFORM STORAGE ACTIONS
            //! ************************************************************************
            // Add specific storage logic here
            // For example: release gripper, servo movements, etc.
            
            Serial.println("Storage actions completed");
            storingComplete = true;
        }
    } else {
        Serial.println("ERROR: Z motor not available");
        setState(IDLE_STATE);
    }
}

void resetStoringState() {
    //! ************************************************************************
    //! RESET STORING STATE FLAGS
    //! ************************************************************************
    storingStateInitialized = false;
    storingComplete = false;
}

void setStoringReferences(FastAccelStepper* motor) {
    //! ************************************************************************
    //! SET REFERENCES TO MOTOR OBJECTS
    //! ************************************************************************
    storingZMotor = motor;
} 