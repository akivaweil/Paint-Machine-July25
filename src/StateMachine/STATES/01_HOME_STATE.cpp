//* ************************************************************************
//* ************************ HOME STATE **********************************
//* ************************************************************************
//! HOME state - Performs homing sequence for loader height
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

//* ************************************************************************
//* ************************ HOME STATE VARIABLES ************************
//* ************************************************************************
static bool homeStateInitialized = false;
static bool homeComplete = false;
static bool homeFound = false;
static bool movingAwayFromHome = false;
static bool testMotionComplete = false;
static FastAccelStepper* homeLoaderHeightMotor = NULL;
static Bounce2::Button* homeLoaderHeightHomeSwitch = NULL;
static ServoControl* homeServo = NULL;
static ServoAccelerationController* homeServoController = NULL;

//* ************************************************************************
//* ************************ HOME STATE FUNCTIONS ************************
//* ************************************************************************

void executeHomeState() {
    //! ************************************************************************
    //! EXECUTE HOME STATE - LOADER HEIGHT HOMING SEQUENCE
    //! ************************************************************************
    
    // Initialize home state on first entry
    if (!homeStateInitialized) {
        Serial.println("=== ENTERING HOME STATE ===");
        Serial.println("Starting loader height homing sequence...");
        homeStateInitialized = true;
        homeComplete = false;
        homeFound = false;
        movingAwayFromHome = false;
        testMotionComplete = false;
        
        //! ************************************************************************
        //! STEP 1: SET HOMING SPEED
        //! ************************************************************************
        if (homeLoaderHeightMotor) {
            homeLoaderHeightMotor->setSpeedInHz(Z_HOMING_SPEED);
            Serial.println("Loader Height Motor homing speed set to: " + String(Z_HOMING_SPEED) + " Hz");
        }
        
        //! ************************************************************************
        //! STEP 2: START MOVING TOWARD HOME
        //! ************************************************************************
        if (homeLoaderHeightMotor) {
            homeLoaderHeightMotor->runBackward();
            Serial.println("Moving toward home switch...");
        }
    }
    
    //! ************************************************************************
    //! STEP 3: CHECK IF HOMING IS COMPLETE
    //! ************************************************************************
    if (homeComplete) {
        //! ************************************************************************
        //! STEP 8: PERFORM TEST MOTION SEQUENCE
        //! ************************************************************************
        if (!testMotionComplete && homeServo && homeServoController) {
            performTestMotionSequence();
            testMotionComplete = true;
        }
        
        // Wait for test motion to complete before transitioning
        if (testMotionComplete) {
            Serial.println("Homing sequence and test motion complete - transitioning to IDLE");
            setState(IDLE_STATE);
            return;
        }
    }
    
    //! ************************************************************************
    //! STEP 4: PERFORM HOMING SEQUENCE
    //! ************************************************************************
    if (homeLoaderHeightMotor && homeLoaderHeightHomeSwitch) {
        // Update the home switch state
        homeLoaderHeightHomeSwitch->update();
        
        // Debug: Print switch state
        static bool lastSwitchState = false;
        bool currentSwitchState = homeLoaderHeightHomeSwitch->read();
        if (currentSwitchState != lastSwitchState) {
            Serial.println("Home switch state changed to: " + String(currentSwitchState ? "TRIGGERED" : "NOT TRIGGERED"));
            lastSwitchState = currentSwitchState;
        }
        
        // Check if home switch is triggered (active high) and we haven't found home yet
        if (currentSwitchState && !homeFound) {
            //! ************************************************************************
            //! STEP 5: HOME SWITCH TRIGGERED - STOP AND SET HOME
            //! ************************************************************************
            homeLoaderHeightMotor->forceStop();
            homeLoaderHeightMotor->setCurrentPosition(0); // Set current position as home (0)
            
            Serial.println("Home switch triggered - loader height homed");
            Serial.println("Current position set to 0");
            
            homeFound = true;
            
            //! ************************************************************************
            //! STEP 6: START MOVING AWAY FROM HOME
            //! ************************************************************************
            Serial.println("Moving " + String(Z_HOME_OFFSET_INCHES) + " inches away from home...");
            
            // Set normal operating speed for moving away
            homeLoaderHeightMotor->setSpeedInHz(Z_MAX_SPEED);
            Serial.println("Motor speed set to: " + String(Z_MAX_SPEED) + " Hz");
            
            // Move to the offset position (2 inches away from home)
            homeLoaderHeightMotor->moveTo(Z_HOME_OFFSET_STEPS);
            Serial.println("Movement command sent to position: " + String(Z_HOME_OFFSET_STEPS) + " steps");
            
            movingAwayFromHome = true;
        }
        
        //! ************************************************************************
        //! STEP 7: CHECK IF MOVEMENT AWAY FROM HOME IS COMPLETE
        //! ************************************************************************
        if (movingAwayFromHome && !homeLoaderHeightMotor->isRunning()) {
            Serial.println("Movement away from home complete");
            Serial.println("Final position: " + String(homeLoaderHeightMotor->getCurrentPosition()) + " steps");
            homeComplete = true;
        }
    }
    
    // Only show error if we don't have the required components
    if (!homeLoaderHeightMotor || !homeLoaderHeightHomeSwitch) {
        Serial.println("ERROR: Loader height motor or home switch not available");
        setState(IDLE_STATE);
    }
}

void performTestMotionSequence() {
    //! ************************************************************************
    //! PERFORM TEST MOTION SEQUENCE AFTER HOMING
    //! ************************************************************************
    Serial.println("=== STARTING TEST MOTION SEQUENCE ===");
    
    if (!homeServo || !homeServoController) {
        Serial.println("ERROR: Servo or servo controller not available for test motion");
        return;
    }
    
    //! ************************************************************************
    //! STEP 1: MOVE SERVO TO 70 DEGREES
    //! ************************************************************************
    Serial.println("Moving servo to 70 degrees...");
    homeServoController->setAccelerationProfile(100, 150);
    homeServoController->moveTo(50);
    while (homeServoController->isMoving()) {
        homeServoController->update();
        delay(10);
    }
    Serial.println("Servo reached 70 degrees");
    
    //! ************************************************************************
    //! STEP 2: MOVE FROM 70 TO 130 WITH ACCEL 10
    //! ************************************************************************
    Serial.println("Moving servo from 70 to 130 degrees with acceleration 10...");
    homeServoController->setAccelerationProfile(500, 3000);
    homeServoController->moveTo(150);
    
    while (homeServoController->isMoving()) {
        homeServoController->update();
        delay(10);
    }
    Serial.println("Servo reached 130 degrees with accel 10");
    
    Serial.println("=== TEST MOTION SEQUENCE COMPLETE ===");
}

void resetHomeState() {
    //! ************************************************************************
    //! RESET HOME STATE FLAGS
    //! ************************************************************************
    homeStateInitialized = false;
    homeComplete = false;
    homeFound = false;
    movingAwayFromHome = false;
    testMotionComplete = false;
}

void setHomeReferences(FastAccelStepper* motor, Bounce2::Button* homeSwitch) {
    //! ************************************************************************
    //! SET REFERENCES TO MOTOR AND SWITCH OBJECTS
    //! ************************************************************************
    homeLoaderHeightMotor = motor;
    homeLoaderHeightHomeSwitch = homeSwitch;
    
    Serial.println("Home motor/switch references set - Motor: " + String(motor ? "VALID" : "NULL") + 
                  ", Switch: " + String(homeSwitch ? "VALID" : "NULL"));
}

void setHomeServoReferences(ServoControl* servo, ServoAccelerationController* controller) {
    //! ************************************************************************
    //! SET REFERENCES TO SERVO AND SERVO CONTROLLER OBJECTS
    //! ************************************************************************
    homeServo = servo;
    homeServoController = controller;
    
    Serial.println("Home servo references set - Servo: " + String(servo ? "VALID" : "NULL") + 
                  ", Controller: " + String(controller ? "VALID" : "NULL"));
} 