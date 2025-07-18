//* ************************************************************************
//* ************************ TEST STATE **********************************
//* ************************************************************************
//! TEST state - Manual testing state for development and debugging
//! This state is entered by pressing the manual start button
//! Provides a safe environment for testing individual components

#include <Arduino.h>
#include "StateMachine.h"
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"
#include <FastAccelStepper.h>

//* ************************************************************************
//* ************************ TEST STATE VARIABLES ************************
//* ************************************************************************
static bool testStateInitialized = false;
static bool testComplete = false;
static FastAccelStepper* testZMotor = NULL;

//* ************************************************************************
//* ************************ TEST STATE FUNCTIONS ************************
//* ************************************************************************

void executeTestState() {
    //! ************************************************************************
    //! EXECUTE TEST STATE - MANUAL TESTING OPERATION
    //! ************************************************************************
    
    // Initialize test state on first entry
    if (!testStateInitialized) {
        Serial.println("=== ENTERING TEST STATE ===");
        Serial.println("Manual testing mode activated");
        Serial.println("Press start button again to exit test mode");
        testStateInitialized = true;
        testComplete = false;
        
        //! ************************************************************************
        //! STEP 1: PERFORM TEST SEQUENCE
        //! ************************************************************************
        Serial.println("Performing test sequence...");
        
        // Add test logic here
        // For example: move motor to test position, activate servo, etc.
        if (testZMotor) {
            Serial.println("Test: Moving Z motor to test position");
            testZMotor->setSpeedInHz(Z_MAX_SPEED);
            
            // Move to a test position (3 inches from home)
            int testPosition = 3 * STEPS_PER_INCH;
            testZMotor->moveTo(testPosition);
            Serial.println("Test: Moving to position: " + String(testPosition) + " steps");
        }
    }
    
    //! ************************************************************************
    //! STEP 2: CHECK IF TEST IS COMPLETE
    //! ************************************************************************
    if (testComplete) {
        Serial.println("Test sequence complete - transitioning to IDLE");
        setState(IDLE_STATE);
        return;
    }
    
    //! ************************************************************************
    //! STEP 3: MONITOR TEST PROGRESS
    //! ************************************************************************
    if (testZMotor) {
        // Check if motor movement is complete
        if (!testZMotor->isRunning()) {
            Serial.println("Test: Motor movement complete");
            Serial.println("Test: Current position: " + String(testZMotor->getCurrentPosition()) + " steps");
            
            // Wait a moment then return to home position
            static unsigned long testDelay = 0;
            if (testDelay == 0) {
                testDelay = millis();
                Serial.println("Test: Waiting 2 seconds before returning to home...");
            }
            
            if (millis() - testDelay > 2000) {
                Serial.println("Test: Returning to home position");
                testZMotor->moveTo(0);
                testDelay = 0;
            }
            
            // Check if we've returned to home
            if (testZMotor->getCurrentPosition() == 0) {
                Serial.println("Test: Returned to home position");
                testComplete = true;
            }
        }
    } else {
        Serial.println("ERROR: Z motor not available for testing");
        setState(IDLE_STATE);
    }
}

void resetTestState() {
    //! ************************************************************************
    //! RESET TEST STATE FLAGS
    //! ************************************************************************
    testStateInitialized = false;
    testComplete = false;
}

void setTestReferences(FastAccelStepper* motor) {
    //! ************************************************************************
    //! SET REFERENCES TO MOTOR OBJECTS
    //! ************************************************************************
    testZMotor = motor;
    Serial.println("Test motor references set - Motor: " + String(motor ? "VALID" : "NULL"));
} 