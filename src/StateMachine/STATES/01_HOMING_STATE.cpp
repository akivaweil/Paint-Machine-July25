//* ************************************************************************
//* ************************ HOMING STATE *********************************
//* ************************************************************************
//! HOMING state - Performs homing sequence for Z-axis
//! Moves toward home switch until triggered, then sets position to 0

#include <Arduino.h>
#include "StateMachine.h"
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"
#include <FastAccelStepper.h>
#include <Bounce2.h>

//* ************************************************************************
//* ************************ HOMING STATE VARIABLES ***********************
//* ************************************************************************
static bool homingStateInitialized = false;
static bool homingComplete = false;
static FastAccelStepper* homingZMotor = NULL;
static Bounce2::Button* homingZHomeSwitch = NULL;

//* ************************************************************************
//* ************************ HOMING STATE FUNCTIONS ***********************
//* ************************************************************************

void executeHomingState() {
    //! ************************************************************************
    //! EXECUTE HOMING STATE - Z-AXIS HOMING SEQUENCE
    //! ************************************************************************
    
    // Initialize homing state on first entry
    if (!homingStateInitialized) {
        Serial.println("=== ENTERING HOMING STATE ===");
        Serial.println("Starting Z-axis homing sequence...");
        homingStateInitialized = true;
        homingComplete = false;
        
        //! ************************************************************************
        //! STEP 1: SET HOMING SPEED
        //! ************************************************************************
        if (homingZMotor) {
            homingZMotor->setSpeedInHz(Z_HOMING_SPEED);
            Serial.println("Z Motor homing speed set to: " + String(Z_HOMING_SPEED) + " Hz");
        }
    }
    
    //! ************************************************************************
    //! STEP 2: CHECK IF HOMING IS COMPLETE
    //! ************************************************************************
    if (homingComplete) {
        Serial.println("Homing sequence complete - transitioning to IDLE");
        setState(IDLE_STATE);
        return;
    }
    
    //! ************************************************************************
    //! STEP 3: PERFORM HOMING SEQUENCE
    //! ************************************************************************
    if (homingZMotor && homingZHomeSwitch) {
        // Start moving in negative direction (toward home)
        if (!homingZMotor->isRunning()) {
            homingZMotor->runBackward();
            Serial.println("Moving toward home switch...");
        }
        
        // Check if home switch is triggered (active high)
        if (homingZHomeSwitch->read()) {
            //! ************************************************************************
            //! STEP 4: HOME SWITCH TRIGGERED - STOP AND SET HOME
            //! ************************************************************************
            homingZMotor->forceStop();
            homingZMotor->setCurrentPosition(0); // Set current position as home (0)
            
            Serial.println("Home switch triggered - Z-axis homed");
            Serial.println("Current position set to 0");
            
            homingComplete = true;
        }
    } else {
        Serial.println("ERROR: Z motor or home switch not available");
        setState(IDLE_STATE);
    }
}

void resetHomingState() {
    //! ************************************************************************
    //! RESET HOMING STATE FLAGS
    //! ************************************************************************
    homingStateInitialized = false;
    homingComplete = false;
}

void setHomingReferences(FastAccelStepper* motor, Bounce2::Button* homeSwitch) {
    //! ************************************************************************
    //! SET REFERENCES TO MOTOR AND SWITCH OBJECTS
    //! ************************************************************************
    homingZMotor = motor;
    homingZHomeSwitch = homeSwitch;
} 