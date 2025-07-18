//* ************************************************************************
//* ************************ HOME STATE **********************************
//* ************************************************************************
//! HOME state - Performs homing sequence for Z-axis
//! Moves toward home switch until triggered, then moves away from home

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
static FastAccelStepper* homeZMotor = NULL;
static Bounce2::Button* homeZHomeSwitch = NULL;
static ServoControl* homeServo = NULL;
static ServoAccelerationController* homeServoController = NULL;

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
        Serial.println("Starting Z-axis homing sequence...");
        homeStateInitialized = true;
        homeComplete = false;
        homeFound = false;
        movingAwayFromHome = false;
        testMotionComplete = false;
        
        //! ************************************************************************
        //! STEP 1: SET HOMING SPEED
        //! ************************************************************************
        if (homeZMotor) {
            homeZMotor->setSpeedInHz(Z_HOMING_SPEED);
            Serial.println("Z Motor homing speed set to: " + String(Z_HOMING_SPEED) + " Hz");
        }
        
        //! ************************************************************************
        //! STEP 2: START MOVING TOWARD HOME
        //! ************************************************************************
        if (homeZMotor) {
            homeZMotor->runBackward();
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
            //! STEP 5: HOME SWITCH TRIGGERED - STOP AND SET HOME
            //! ************************************************************************
            homeZMotor->forceStop();
            homeZMotor->setCurrentPosition(0); // Set current position as home (0)
            
            Serial.println("Home switch triggered - Z-axis homed");
            Serial.println("Current position set to 0");
            
            homeFound = true;
            
            //! ************************************************************************
            //! STEP 6: START MOVING AWAY FROM HOME
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
        //! STEP 7: CHECK IF MOVEMENT AWAY FROM HOME IS COMPLETE
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

void performTestMotionSequence() {
    //! ************************************************************************
    //! PERFORM TEST MOTION SEQUENCE AFTER HOMING
    //! ************************************************************************
    Serial.println("=== STARTING TEST MOTION SEQUENCE ===");
    
    if (!homeServo || !homeServoController) {
        Serial.println("ERROR: Servo or servo controller not available for test motion");
        return;
    }
    
    // Configure servo controller for smooth motion
    homeServoController->setAccelerationProfile(375, 2250); // 75% of original values
    homeServoController->setSmoothingFactor(0.15); // 15% smoothing factor
    homeServoController->setUpdateInterval(5000); // 5ms update interval (200Hz)
    
    //! ************************************************************************
    //! STEP 1: MOVE SERVO TO 50 DEGREES
    //! ************************************************************************
    Serial.println("Moving servo to 50 degrees (smooth motion enabled)...");
    homeServoController->moveTo(50);
    while (homeServoController->isMoving()) {
        homeServoController->update();
        delay(10);
    }
    Serial.println("Servo reached 50 degrees");
    
    //! ************************************************************************
    //! STEP 2: MOVE FROM 50 TO 150 DEGREES WITH IMPROVED SETTINGS
    //! ************************************************************************
    Serial.println("Moving servo from 50 to 150 degrees (smooth motion enabled)...");
    homeServoController->moveTo(150);
    
    unsigned long startTime = millis();
    while (homeServoController->isMoving()) {
        homeServoController->update();
        delay(10);
        
        // Add timeout protection
        if (millis() - startTime > 10000) { // 10 second timeout
            Serial.println("WARNING: Servo movement timeout - forcing completion");
            break;
        }
    }
    Serial.println("Servo reached 150 degrees");
    
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
    homeZMotor = motor;
    homeZHomeSwitch = homeSwitch;
    
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