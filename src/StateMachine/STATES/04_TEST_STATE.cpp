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
#include "ServoAccelerationController.h"

//* ************************************************************************
//* ************************ TEST STATE VARIABLES ************************
//* ************************************************************************
static bool testStateInitialized = false;
static bool testComplete = false;
static FastAccelStepper* testZMotor = NULL;
static ServoAccelerationController* testServoController = NULL;

// Test sequence variables
static int testStep = 0;
static bool motorMoving = false;
static bool servoMoving = false;
static unsigned long testDelay = 0;
static unsigned long stepStartTime = 0; // Timeout tracking

// Test positions
static const int TEST_POSITION_1_INCHES = 10;  // First position: 10 inches
static const int TEST_POSITION_2_INCHES = 1;   // Second position: 1 inch (home offset)
static const int SERVO_POSITION_1_DEGREES = 70; // First servo position: 70 degrees
static const int SERVO_POSITION_2_DEGREES = 130; // Second servo position: 130 degrees

// Servo test settings
static const int SERVO_TEST_ACCEL = 500;    // Servo acceleration for test
static const int SERVO_TEST_MAX_SPEED = 3000; // Servo max speed for test

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
        Serial.println("Test sequence: 10\" @ 70° -> 1\" @ 130°");
        testStateInitialized = true;
        testComplete = false;
            testStep = 0;
    motorMoving = false;
    servoMoving = false;
    testDelay = 0;
    stepStartTime = 0;
        
        //! ************************************************************************
        //! STEP 1: START TEST SEQUENCE
        //! ************************************************************************
        Serial.println("Starting test sequence...");
        testStep = 1;
    }
    
    //! ************************************************************************
    //! STEP 2: UPDATE SERVO CONTROLLER
    //! ************************************************************************
    if (testServoController) {
        testServoController->update();
    }
    
    //! ************************************************************************
    //! STEP 3: EXECUTE TEST SEQUENCE
    //! ************************************************************************
    switch (testStep) {
        case 1: // Move to 10 inches with servo at 70 degrees
            if (!motorMoving && !servoMoving) {
                Serial.println("Test Step 1: Moving to 10 inches with servo at 70°");
                stepStartTime = millis(); // Start timeout timer
                
                // Move motor to 10 inches
                if (testZMotor) {
                    int targetPosition = TEST_POSITION_1_INCHES * STEPS_PER_INCH;
                    testZMotor->setSpeedInHz(Z_MAX_SPEED);
                    testZMotor->setAcceleration(Z_ACCELERATION);
                    testZMotor->moveTo(targetPosition);
                    motorMoving = true;
                    Serial.println("Motor moving to: " + String(targetPosition) + " steps (" + String(TEST_POSITION_1_INCHES) + " inches)");
                }
                
                // Move servo to 70 degrees with specified acceleration and speed
                if (testServoController) {
                    testServoController->setAccelerationProfile(SERVO_TEST_ACCEL, SERVO_TEST_MAX_SPEED);
                    testServoController->moveTo(SERVO_POSITION_1_DEGREES);
                    servoMoving = true;
                    Serial.println("Servo moving to: " + String(SERVO_POSITION_1_DEGREES) + " degrees (accel: " + String(SERVO_TEST_ACCEL) + ", max speed: " + String(SERVO_TEST_MAX_SPEED) + ")");
                }
            }
            
            // Check if both movements are complete
            if (testZMotor && testServoController) {
                bool motorComplete = !testZMotor->isRunning();
                bool servoComplete = testServoController->hasReachedTarget() || 
                                   (abs(testServoController->getCurrentAngle() - SERVO_POSITION_1_DEGREES) < 2.0 && 
                                    !testServoController->isMoving());
                
                // Debug output
                static unsigned long lastDebugTime = 0;
                if (millis() - lastDebugTime > 1000) { // Print every second
                    Serial.println("Debug - Motor running: " + String(motorComplete ? "NO" : "YES") + 
                                   ", Servo target reached: " + String(servoComplete ? "YES" : "NO") +
                                   ", Servo angle: " + String(testServoController->getCurrentAngle()) +
                                   ", Target: " + String(SERVO_POSITION_1_DEGREES) +
                                   ", State: " + String(testServoController->getMotionState()) +
                                   ", Distance: " + String(abs(testServoController->getCurrentAngle() - SERVO_POSITION_1_DEGREES)));
                    lastDebugTime = millis();
                }
                
                // Check for timeout (30 seconds)
                if (millis() - stepStartTime > 30000) {
                    Serial.println("WARNING: Step 1 timeout - forcing completion");
                    motorMoving = false;
                    servoMoving = false;
                    testStep = 2;
                    testDelay = millis();
                    Serial.println("Waiting 2 seconds before next movement...");
                }
                else if (motorComplete && servoComplete) {
                    Serial.println("Test Step 1 complete - Motor at: " + String(testZMotor->getCurrentPosition()) + " steps");
                    motorMoving = false;
                    servoMoving = false;
                    testStep = 3; // Go directly to step 3, skip delay
                    Serial.println("Moving directly to Step 2 (no delay)...");
                }
            }
            break;
            
        case 2: // Removed - no longer needed
            break;
            
        case 3: // Move to 1 inch (home offset) with servo at 130 degrees
            if (!motorMoving && !servoMoving) {
                Serial.println("Test Step 2: Moving to 1 inch (home offset) with servo at 130°");
                stepStartTime = millis(); // Start timeout timer
                
                // Move motor to 2 inches
                if (testZMotor) {
                    int targetPosition = TEST_POSITION_2_INCHES * STEPS_PER_INCH;
                    testZMotor->setSpeedInHz(Z_MAX_SPEED);
                    testZMotor->setAcceleration(Z_ACCELERATION);
                    testZMotor->moveTo(targetPosition);
                    motorMoving = true;
                    Serial.println("Motor moving to: " + String(targetPosition) + " steps (" + String(TEST_POSITION_2_INCHES) + " inches)");
                }
                
                // Move servo to 130 degrees with specified acceleration and speed
                if (testServoController) {
                    testServoController->setAccelerationProfile(SERVO_TEST_ACCEL, SERVO_TEST_MAX_SPEED);
                    testServoController->moveTo(SERVO_POSITION_2_DEGREES);
                    servoMoving = true;
                    Serial.println("Servo moving to: " + String(SERVO_POSITION_2_DEGREES) + " degrees (accel: " + String(SERVO_TEST_ACCEL) + ", max speed: " + String(SERVO_TEST_MAX_SPEED) + ")");
                }
            }
            
            // Check if both movements are complete
            if (testZMotor && testServoController) {
                bool motorComplete = !testZMotor->isRunning();
                bool servoComplete = testServoController->hasReachedTarget() || 
                                   (abs(testServoController->getCurrentAngle() - SERVO_POSITION_2_DEGREES) < 2.0 && 
                                    !testServoController->isMoving());
                
                // Debug output
                static unsigned long lastDebugTime2 = 0;
                if (millis() - lastDebugTime2 > 1000) { // Print every second
                    Serial.println("Debug Step 2 - Motor running: " + String(motorComplete ? "NO" : "YES") + 
                                   ", Servo target reached: " + String(servoComplete ? "YES" : "NO") +
                                   ", Servo angle: " + String(testServoController->getCurrentAngle()) +
                                   ", Target: " + String(SERVO_POSITION_2_DEGREES) +
                                   ", State: " + String(testServoController->getMotionState()) +
                                   ", Distance: " + String(abs(testServoController->getCurrentAngle() - SERVO_POSITION_2_DEGREES)));
                    lastDebugTime2 = millis();
                }
                
                // Check for timeout (30 seconds)
                if (millis() - stepStartTime > 30000) {
                    Serial.println("WARNING: Step 2 timeout - forcing completion");
                    motorMoving = false;
                    servoMoving = false;
                    testStep = 4;
                    testDelay = millis();
                    Serial.println("Waiting 2 seconds before returning to home...");
                }
                else if (motorComplete && servoComplete) {
                    Serial.println("Test Step 2 complete - Motor at: " + String(testZMotor->getCurrentPosition()) + " steps");
                    motorMoving = false;
                    servoMoving = false;
                    testStep = 5; // Go directly to home return, skip delay
                    Serial.println("Moving directly to home position (no delay)...");
                }
            }
            break;
            
        case 4: // Removed - no longer needed
            break;
            
        case 5: // Return to home position
            if (!motorMoving) {
                if (testZMotor) {
                    testZMotor->setSpeedInHz(Z_MAX_SPEED);
                    testZMotor->setAcceleration(Z_ACCELERATION);
                    testZMotor->moveTo(0);
                    motorMoving = true;
                    Serial.println("Motor returning to home position");
                }
            }
            
            // Check if motor has returned to home
            if (testZMotor && !testZMotor->isRunning()) {
                if (testZMotor->getCurrentPosition() == 0) {
                    Serial.println("Test sequence complete - returned to home");
                    testComplete = true;
                }
            }
            break;
    }
    
    //! ************************************************************************
    //! STEP 4: CHECK IF TEST IS COMPLETE
    //! ************************************************************************
    if (testComplete) {
        Serial.println("Test sequence complete - transitioning to IDLE");
        setState(IDLE_STATE);
        return;
    }
}

void resetTestState() {
    //! ************************************************************************
    //! RESET TEST STATE FLAGS
    //! ************************************************************************
    testStateInitialized = false;
    testComplete = false;
    testStep = 0;
    motorMoving = false;
    servoMoving = false;
    testDelay = 0;
}

void setTestReferences(FastAccelStepper* motor, ServoAccelerationController* servoController) {
    //! ************************************************************************
    //! SET REFERENCES TO MOTOR AND SERVO CONTROLLER OBJECTS
    //! ************************************************************************
    testZMotor = motor;
    testServoController = servoController;
    Serial.println("Test references set - Motor: " + String(motor ? "VALID" : "NULL") + 
                   ", ServoController: " + String(servoController ? "VALID" : "NULL"));
} 