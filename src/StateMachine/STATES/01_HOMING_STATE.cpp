//* ************************************************************************
//* ************************ HOMING STATE *********************************
//* ************************************************************************
//! HOMING state - Performs homing sequence for Z-axis
//! Moves toward home switch until triggered, then moves away from home

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
static bool homeFound = false;
static bool movingAwayFromHome = false;
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
        homeFound = false;
        movingAwayFromHome = false;
        
        //! ************************************************************************
        //! STEP 1: SET HOMING SPEED
        //! ************************************************************************
        if (homingZMotor) {
            homingZMotor->setSpeedInHz(Z_HOMING_SPEED);
            Serial.println("Z Motor homing speed set to: " + String(Z_HOMING_SPEED) + " Hz");
        }
        
        //! ************************************************************************
        //! STEP 2: START MOVING TOWARD HOME
        //! ************************************************************************
        if (homingZMotor) {
            homingZMotor->runBackward();
            Serial.println("Moving toward home switch...");
        }
    }
    
    //! ************************************************************************
    //! STEP 3: CHECK IF HOMING IS COMPLETE
    //! ************************************************************************
    if (homingComplete) {
        Serial.println("Homing sequence complete - transitioning to IDLE");
        setState(IDLE_STATE);
        return;
    }
    
    //! ************************************************************************
    //! STEP 4: PERFORM HOMING SEQUENCE
    //! ************************************************************************
    if (homingZMotor && homingZHomeSwitch) {
        // Update the home switch state
        homingZHomeSwitch->update();
        
        // Debug: Print switch state
        static bool lastSwitchState = false;
        bool currentSwitchState = homingZHomeSwitch->read();
        if (currentSwitchState != lastSwitchState) {
            Serial.println("Home switch state changed to: " + String(currentSwitchState ? "TRIGGERED" : "NOT TRIGGERED"));
            lastSwitchState = currentSwitchState;
        }
        
        // Check if home switch is triggered (active high) and we haven't found home yet
        if (currentSwitchState && !homeFound) {
            //! ************************************************************************
            //! STEP 5: HOME SWITCH TRIGGERED - STOP AND SET HOME
            //! ************************************************************************
            homingZMotor->forceStop();
            homingZMotor->setCurrentPosition(0); // Set current position as home (0)
            
            Serial.println("Home switch triggered - Z-axis homed");
            Serial.println("Current position set to 0");
            
            homeFound = true;
            
            //! ************************************************************************
            //! STEP 6: START MOVING AWAY FROM HOME
            //! ************************************************************************
            Serial.println("Moving " + String(Z_HOME_OFFSET_INCHES) + " inches away from home...");
            
            // Set normal operating speed for moving away
            homingZMotor->setSpeedInHz(Z_MAX_SPEED);
            Serial.println("Motor speed set to: " + String(Z_MAX_SPEED) + " Hz");
            
            // Move to the offset position (2 inches away from home)
            homingZMotor->moveTo(Z_HOME_OFFSET_STEPS);
            Serial.println("Movement command sent to position: " + String(Z_HOME_OFFSET_STEPS) + " steps");
            
            movingAwayFromHome = true;
        }
        
        //! ************************************************************************
        //! STEP 7: CHECK IF MOVEMENT AWAY FROM HOME IS COMPLETE
        //! ************************************************************************
        if (movingAwayFromHome && !homingZMotor->isRunning()) {
            Serial.println("Movement away from home complete");
            Serial.println("Final position: " + String(homingZMotor->getCurrentPosition()) + " steps");
            homingComplete = true;
        }
    }
    
    // Only show error if we don't have the required components
    if (!homingZMotor || !homingZHomeSwitch) {
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
    homeFound = false;
    movingAwayFromHome = false;
}

void setHomingReferences(FastAccelStepper* motor, Bounce2::Button* homeSwitch) {
    //! ************************************************************************
    //! SET REFERENCES TO MOTOR AND SWITCH OBJECTS
    //! ************************************************************************
    homingZMotor = motor;
    homingZHomeSwitch = homeSwitch;
    
    Serial.println("Homing motor/switch references set - Motor: " + String(motor ? "VALID" : "NULL") + 
                  ", Switch: " + String(homeSwitch ? "VALID" : "NULL"));
} 