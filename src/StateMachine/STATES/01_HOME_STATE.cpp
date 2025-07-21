//* ************************************************************************
//* ************************ HOME STATE **********************************
//* ************************************************************************
//! HOME state - Performs homing sequence for Z-axis
//! Moves toward home switch until triggered, then moves away from home
//!
//! ⚠️  IMPORTANT: DO NOT ADD TIMEOUT CHECKS TO THIS STATE MACHINE
//! ⚠️  Timeout checks were removed because they cause freezing issues
//! ⚠️  Only add timeout checks if specifically instructed to do so
//! ⚠️  The servo controller and motor have their own completion detection

#include <Arduino.h>
#include "StateMachine.h"
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"
#include <FastAccelStepper.h>
#include <Bounce2.h>
#include "ServoControl.h"
#include "ServoAccelerationController.h"
#include "LoaderForkStepper.h"

//* ************************************************************************
//* ************************ HOME STATE VARIABLES ************************
//* ************************************************************************
static bool homeStateInitialized = false;
static bool homeComplete = false;
static bool homeFound = false;
static bool movingAwayFromHome = false;
static bool forkHomingComplete = false; // Track if fork homing is complete

static FastAccelStepper* homeZMotor = NULL;
static Bounce2::Button* homeZHomeSwitch = NULL;
static LoaderForkStepper* homeForkStepper = NULL; // Loader fork stepper for homing


//* ************************************************************************
//* ************************ HOME STATE FUNCTIONS ************************
//* ************************************************************************

void executeHomeState() {
    //! ************************************************************************
    //! EXECUTE HOME STATE - Z-AXIS HOMING SEQUENCE
    //! ************************************************************************
    
    // Initialize home state on first entry
    if (!homeStateInitialized) {
        Serial.println("=== ENTERING HOME STATE ===");
        Serial.println("Starting homing sequence...");
        homeStateInitialized = true;
        homeComplete = false;
        homeFound = false;
        movingAwayFromHome = false;
        forkHomingComplete = false;
        
        //! ************************************************************************
        //! STEP 0: HOME THE LOADER FORK FIRST (BLOCKING)
        //! ************************************************************************
        if (homeForkStepper) {
            Serial.println("Step 0: Homing loader fork...");
            homeForkStepper->homeFork();
            forkHomingComplete = true;
            Serial.println("Step 0: Loader fork homed successfully.");
        } else {
            Serial.println("Step 0: No fork stepper available, skipping fork homing.");
            forkHomingComplete = true;
        }
        
        //! ************************************************************************
        //! STEP 1: START Z-AXIS HOMING SEQUENCE
        //! ************************************************************************
        Serial.println("Step 1: Starting Z-axis homing sequence...");
        
        //! ************************************************************************
        //! STEP 2: SET HOMING SPEED
        //! ************************************************************************
        if (homeZMotor) {
            homeZMotor->setSpeedInHz(Z_HOMING_SPEED);
            Serial.println("Z Motor homing speed set to: " + String(Z_HOMING_SPEED) + " Hz");
        }
        
        //! ************************************************************************
        //! STEP 3: START MOVING TOWARD HOME
        //! ************************************************************************
        if (homeZMotor) {
            homeZMotor->runBackward();
            Serial.println("Moving toward home switch...");
        }
    }
    
    //! ************************************************************************
    //! STEP 4: CHECK IF HOMING IS COMPLETE
    //! ************************************************************************
    if (homeComplete) {
        Serial.println("Homing sequence complete - transitioning to IDLE");
        setState(IDLE_STATE);
        return;
    }
    
    //! ************************************************************************
    //! STEP 5: PERFORM HOMING SEQUENCE
    //! ************************************************************************
    if (homeZMotor && homeZHomeSwitch) {
        // Update the home switch state
        homeZHomeSwitch->update();
        
        // Debug: Print switch state
        static bool lastSwitchState = false;
        bool currentSwitchState = homeZHomeSwitch->read();
        if (currentSwitchState != lastSwitchState) {
            Serial.println("Home switch state changed to: " + String(currentSwitchState ? "TRIGGERED" : "NOT TRIGGERED"));
            lastSwitchState = currentSwitchState;
        }
        
        // Check if home switch is triggered (active high) and we haven't found home yet
        if (currentSwitchState && !homeFound) {
            //! ************************************************************************
            //! STEP 6: HOME SWITCH TRIGGERED - STOP AND SET HOME
            //! ************************************************************************
            homeZMotor->forceStop();
            homeZMotor->setCurrentPosition(0); // Set current position as home (0)
            
            Serial.println("Home switch triggered - Z-axis homed");
            Serial.println("Current position set to 0");
            
            homeFound = true;
            
            //! ************************************************************************
            //! STEP 7: START MOVING AWAY FROM HOME
            //! ************************************************************************
            Serial.println("Moving " + String(Z_HOME_OFFSET_INCHES) + " inches away from home...");
            
            // Set normal operating speed for moving away
            homeZMotor->setSpeedInHz(Z_MAX_SPEED);
            Serial.println("Motor speed set to: " + String(Z_MAX_SPEED) + " Hz");
            
            // Move to the offset position (2 inches away from home)
            homeZMotor->moveTo(Z_HOME_OFFSET_STEPS);
            Serial.println("Movement command sent to position: " + String(Z_HOME_OFFSET_STEPS) + " steps");
            
            movingAwayFromHome = true;
        }
        
        //! ************************************************************************
        //! STEP 8: CHECK IF MOVEMENT AWAY FROM HOME IS COMPLETE
        //! ************************************************************************
        if (movingAwayFromHome && !homeZMotor->isRunning()) {
            Serial.println("Movement away from home complete");
            Serial.println("Final position: " + String(homeZMotor->getCurrentPosition()) + " steps");
            homeComplete = true;
        }
    }
    
    // Only show error if we don't have the required components
    if (!homeZMotor || !homeZHomeSwitch) {
        Serial.println("ERROR: Z motor or home switch not available");
        setState(IDLE_STATE);
    }
}



void resetHomeState() {
    //! ************************************************************************
    //! RESET HOME STATE FLAGS
    //! ************************************************************************
    homeStateInitialized = false;
    homeComplete = false;
    homeFound = false;
    movingAwayFromHome = false;
    forkHomingComplete = false;
}

void setHomeReferences(FastAccelStepper* motor, Bounce2::Button* homeSwitch) {
    //! ************************************************************************
    //! SET REFERENCES TO MOTOR AND SWITCH OBJECTS
    //! ************************************************************************
    homeZMotor = motor;
    homeZHomeSwitch = homeSwitch;
    Serial.println("Home motor/switch references set - Motor: " + String(motor ? "VALID" : "NULL") + ", Switch: " + String(homeSwitch ? "VALID" : "NULL"));
}

// Overload to set loader fork stepper reference as well
void setHomeReferences(FastAccelStepper* motor, Bounce2::Button* homeSwitch, LoaderForkStepper* forkStepper) {
    homeZMotor = motor;
    homeZHomeSwitch = homeSwitch;
    homeForkStepper = forkStepper;
    Serial.println("Home motor/switch/fork references set - Motor: " + String(motor ? "VALID" : "NULL") + ", Switch: " + String(homeSwitch ? "VALID" : "NULL") + ", Fork: " + String(forkStepper ? "VALID" : "NULL"));
}

 