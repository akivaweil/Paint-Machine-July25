//* ************************************************************************
//* ************************ TEST STATE **********************************
//* ************************************************************************
//! TEST state - Manual testing state for development and debugging
//! This state is entered by pressing the manual start button
//! Provides a safe environment for testing individual components
//!
//! ⚠️  IMPORTANT: DO NOT ADD TIMEOUT CHECKS TO THIS STATE MACHINE
//! ⚠️  Timeout checks were removed because they cause freezing issues
//! ⚠️  Only add timeout checks if specifically instructed to do so
//! ⚠️  The servo controller and motor have their own completion detection

#include <Arduino.h>
#include "StateMachine.h"
#include "Config/Pins_Definitions.h"
#include <FastAccelStepper.h>
#include "ServoAccelerationController.h"
#include "CylinderControl.h"

//* ************************************************************************
//* ************************ TEST POSITION CONFIGURATION *******************
//* ************************************************************************
// Test position structure for height and angle settings
struct TestPosition {
    float height_inches;     // Height position in inches
    float angle_degrees;     // Servo angle in degrees
    const char* name;        // Position name for identification
};

// Mechanical settings needed for test calculations
static const int STEPS_PER_REV = 400;       // Standard stepper motor steps per revolution (1.8° per step)
static const int PULLEY_TEETH = 20;         // GT2 pulley teeth count (20-tooth pulley)
static const float BELT_PITCH = 2.0;        // GT2 belt pitch in mm
static const float STEPS_PER_MM = ((float)STEPS_PER_REV / (PULLEY_TEETH * BELT_PITCH));
static const float STEPS_PER_INCH = (STEPS_PER_MM * 25.4);

// Z-axis position settings needed for test
static const float Z_HOME_OFFSET_INCHES = 1.0;    // Distance to move away from home switch after homing
static const int Z_HOME_OFFSET_STEPS = (int)(Z_HOME_OFFSET_INCHES * STEPS_PER_INCH);

// Test sequence positions (5 positions total)
static const TestPosition TEST_POSITIONS[5] = {
    {1.1, 90.0, "Position 1 - High"},      // Position 1: 25 inches, 0 degrees
    {3.0, 45.0, "Position 2 - Mid-High"}, // Position 2: 15 inches, 45 degrees
    {20.0, 90.0, "Position 3 - Center"},   // Position 3: 10 inches, 90 degrees
    {25.0, 135.0, "Position 4 - Mid-Low"},  // Position 4: 5 inches, 135 degrees
    {4.0, 120.0, "Position 5 - Low"}       // Position 5: 1 inch, 180 degrees
};

//* ************************************************************************
//* ************************ TEST STATE VARIABLES ************************
//* ************************************************************************
static bool testStateInitialized = false;
static bool testComplete = false;
static FastAccelStepper* testZMotor = NULL;
static ServoAccelerationController* testServoController = NULL;
static CylinderControl* testCylinder = NULL;

// Test sequence variables
static bool motorMoving = false;
static bool servoMoving = false;
static bool cylinderOperating = false;
static unsigned long testDelay = 0;
static unsigned long cylinderDelay = 0;
static int currentPositionIndex = 0;  // Current position in the test sequence
static int cylinderStep = 0;  // Current step in cylinder sequence (0-3)

// Servo test settings
static const int SERVO_TEST_ACCEL = 300;    // Servo acceleration for test
static const int SERVO_TEST_MAX_SPEED = 4000; // Servo max speed for test

// Stepper motor test settings
static const float Z_TEST_MAX_SPEED = 20000;     // Stepper max speed for test (steps/sec)
static const float Z_TEST_ACCELERATION = 15000;   // Stepper acceleration for test (steps/sec^2)

// Cylinder sequence settings
static const unsigned long CYLINDER_WAIT_TIME = 1000;  // 1 second wait time
static const float CYLINDER_MOVE_DISTANCE = 0.5;       // 0.5 inches up movement
static const float CYLINDER_MOVE_SPEED = 1000;         // Stepper speed for cylinder up movement (steps/sec)
static const float CYLINDER_MOVE_ACCELERATION = 3000;  // Stepper acceleration for cylinder up movement (steps/sec^2)

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
        Serial.println("Test sequence: 5 positions with height and angle settings");
        testStateInitialized = true;
        testComplete = false;
        currentPositionIndex = 0;
        motorMoving = false;
        servoMoving = false;
        testDelay = 0;
        
        //! ************************************************************************
        //! STEP 1: SYNCHRONIZE SERVO CONTROLLER
        //! ************************************************************************
        if (testServoController) {
            // Force servo to a known position and synchronize controller
            Serial.println("Synchronizing servo controller...");
            
            // First, stop any ongoing movement
            testServoController->stop();
            delay(50);
            
            // Set servo to 90 degrees (center position) and synchronize
            testServoController->setCurrentAngle(90.0);
            
            // Small delay to ensure servo is stable
            delay(200);
            
            Serial.println("Servo controller synchronized at 90°");
        }
        
        //! ************************************************************************
        //! STEP 2: START TEST SEQUENCE
        //! ************************************************************************
        Serial.println("Starting 5-position test sequence...");
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
    // Check if we've completed all positions
    if (currentPositionIndex >= 5) {
        // Move back to home offset position
        if (!motorMoving) {
            if (testZMotor) {
                testZMotor->setSpeedInHz(Z_TEST_MAX_SPEED);
                testZMotor->setAcceleration(Z_TEST_ACCELERATION);
                testZMotor->moveTo(Z_HOME_OFFSET_STEPS);
                motorMoving = true;
                Serial.println("Returning to home offset position: " + String(Z_HOME_OFFSET_STEPS) + " steps (" + String(Z_HOME_OFFSET_INCHES) + " inches)");
            }
        }
        
        // Check if motor has returned to home offset position
        if (testZMotor && !testZMotor->isRunning()) {
            if (testZMotor->getCurrentPosition() == Z_HOME_OFFSET_STEPS) {
                Serial.println("Test sequence complete - returned to home offset position");
                testComplete = true;
            }
        }
    } else {
        // Execute current position
        if (!motorMoving && !servoMoving) {
            TestPosition currentPos = TEST_POSITIONS[currentPositionIndex];
            Serial.println("Test Position " + String(currentPositionIndex + 1) + ": " + String(currentPos.name));
            Serial.println("Moving to " + String(currentPos.height_inches) + " inches with servo at " + String(currentPos.angle_degrees) + "°");
            
            // Move motor to target height
            if (testZMotor) {
                int targetPosition = (int)(currentPos.height_inches * STEPS_PER_INCH);
                testZMotor->setSpeedInHz(Z_TEST_MAX_SPEED);
                testZMotor->setAcceleration(Z_TEST_ACCELERATION);
                testZMotor->moveTo(targetPosition);
                motorMoving = true;
                Serial.println("Motor moving to: " + String(targetPosition) + " steps (" + String(currentPos.height_inches) + " inches)");
            }
            
            // Move servo to target angle
            if (testServoController) {
                // Check if we actually need to move the servo
                float currentAngle = testServoController->getCurrentAngle();
                float targetAngle = currentPos.angle_degrees;
                float angleDifference = abs(currentAngle - targetAngle);
                
                if (angleDifference > 0.5) { // Only move if more than 0.5 degrees difference
                    testServoController->setAccelerationProfile(SERVO_TEST_ACCEL, SERVO_TEST_MAX_SPEED);
                    testServoController->moveTo(targetAngle);
                    servoMoving = true;
                    Serial.println("Servo moving from " + String(currentAngle) + "° to " + String(targetAngle) + "° (accel: " + String(SERVO_TEST_ACCEL) + ", max speed: " + String(SERVO_TEST_MAX_SPEED) + ")");
                } else {
                    Serial.println("Servo already at target position: " + String(targetAngle) + "° (current: " + String(currentAngle) + "°)");
                    servoMoving = false; // No movement needed
                }
            }
        }
        
        // Check if motor movement is complete and wait for servo time
        if (testZMotor) {
            bool motorComplete = !testZMotor->isRunning();
            
            // Use servo controller's built-in completion time calculation
            if (motorComplete && !servoMoving && testServoController) {
                TestPosition currentPos = TEST_POSITIONS[currentPositionIndex];
                float targetAngle = currentPos.angle_degrees;
                
                // Check if servo movement is needed
                float currentAngle = testServoController->getCurrentAngle();
                float angleDifference = abs(currentAngle - targetAngle);
                
                if (angleDifference > 0.5) {
                    // Ensure servo controller is in a clean state before starting movement
                    if (testServoController->getMotionState() == ServoAccelerationController::IDLE) {
                        // Get completion time from servo controller
                        unsigned long completionTime = testServoController->getMoveCompletionTime();
                        testDelay = completionTime;
                        servoMoving = true;
                        
                        unsigned long waitTime = testServoController->calculateMoveTimeToTarget(targetAngle);
                        Serial.println("Servo movement using built-in calculation: " + String(waitTime) + "ms");
                        Serial.println("Waiting until: " + String(testDelay) + " (current: " + String(millis()) + ")");
                    } else {
                        // If servo is not idle, wait a bit more
                        Serial.println("Servo not in idle state, waiting for stabilization...");
                        delay(50);
                    }
                } else {
                    // No servo movement needed, proceed immediately
                    Serial.println("No servo movement needed - already at target position");
                    servoMoving = false;
                }
            }
            
            // Debug output
            static unsigned long lastDebugTime = 0;
            if (millis() - lastDebugTime > 1000) { // Print every second
                TestPosition currentPos = TEST_POSITIONS[currentPositionIndex];
                unsigned long timeRemaining = testServoController ? testServoController->getRemainingMoveTime() : 0;
                Serial.println("Debug Position " + String(currentPositionIndex + 1) + " - Motor running: " + String(motorComplete ? "NO" : "YES") + 
                               ", Servo waiting: " + String(servoMoving ? "YES" : "NO") +
                               ", Time remaining: " + String(timeRemaining) + "ms" +
                               ", Servo angle: " + String(testServoController ? testServoController->getCurrentAngle() : 0.0) +
                               ", Target: " + String(currentPos.angle_degrees) +
                               ", Move complete: " + String(testServoController ? (testServoController->isMoveComplete() ? "YES" : "NO") : "N/A"));
                lastDebugTime = millis();
            }
            
                    // Move to next position when motor is complete AND servo move is complete
        if (motorComplete && (!servoMoving || (testServoController && testServoController->isMoveComplete()))) {
            // Additional check to ensure servo has actually reached the target
            if (testServoController) {
                TestPosition currentPos = TEST_POSITIONS[currentPositionIndex];
                float currentAngle = testServoController->getCurrentAngle();
                float targetAngle = currentPos.angle_degrees;
                float angleDifference = abs(currentAngle - targetAngle);
                
                if (angleDifference > 2.0) { // If more than 2 degrees off target
                    Serial.println("Servo position check failed - Current: " + String(currentAngle) + "°, Target: " + String(targetAngle) + "°, Difference: " + String(angleDifference) + "°");
                    // Wait a bit more for servo to settle
                    delay(100);
                    return; // Don't proceed yet
                }
                
                Serial.println("Position " + String(currentPositionIndex + 1) + " servo check passed - Current: " + String(currentAngle) + "°, Target: " + String(targetAngle) + "°");
            }
            
            if (!cylinderOperating) {
                // Start cylinder sequence
                TestPosition currentPos = TEST_POSITIONS[currentPositionIndex];
                Serial.println("Position " + String(currentPositionIndex + 1) + " complete - Starting cylinder sequence");
                cylinderOperating = true;
                cylinderStep = 0;
                cylinderDelay = 0;
            }
        }
        }
        
        //! ************************************************************************
        //! STEP 3A: EXECUTE CYLINDER SEQUENCE
        //! ************************************************************************
        if (cylinderOperating) {
            unsigned long currentTime = millis();
            
            switch (cylinderStep) {
                case 0: // Extend cylinder
                    if (testCylinder) {
                        Serial.println("Cylinder Step 0: Extending cylinder");
                        testCylinder->extend();
                        cylinderDelay = currentTime + CYLINDER_WAIT_TIME;
                        cylinderStep = 1;
                    }
                    break;
                    
                case 1: // Wait 1 second, then move motor up 0.5 inches
                    if (currentTime >= cylinderDelay) {
                        if (testZMotor) {
                            int currentPos = testZMotor->getCurrentPosition();
                            int targetPos = currentPos + (int)(CYLINDER_MOVE_DISTANCE * STEPS_PER_INCH);
                            Serial.println("Cylinder Step 1: Moving motor up " + String(CYLINDER_MOVE_DISTANCE) + " inches");
                            Serial.println("From: " + String(currentPos) + " steps to: " + String(targetPos) + " steps");
                            Serial.println("Cylinder move speed: " + String(CYLINDER_MOVE_SPEED) + " steps/sec, accel: " + String(CYLINDER_MOVE_ACCELERATION) + " steps/sec²");
                            testZMotor->setSpeedInHz(CYLINDER_MOVE_SPEED);
                            testZMotor->setAcceleration(CYLINDER_MOVE_ACCELERATION);
                            testZMotor->moveTo(targetPos);
                            cylinderStep = 2;
                        }
                    }
                    break;
                    
                case 2: // Wait for motor to complete, then retract cylinder
                    if (testZMotor && !testZMotor->isRunning()) {
                        if (testCylinder) {
                            Serial.println("Cylinder Step 2: Retracting cylinder");
                            testCylinder->retract();
                            cylinderDelay = currentTime + CYLINDER_WAIT_TIME;
                            cylinderStep = 3;
                        }
                    }
                    break;
                    
                case 3: // Wait 1 second, then move to next position
                    if (currentTime >= cylinderDelay) {
                        Serial.println("Cylinder sequence complete - moving to next position");
                        cylinderOperating = false;
                        motorMoving = false;
                        servoMoving = false;
                        currentPositionIndex++; // Move to next position
                        Serial.println("Moving to next position...");
                    }
                    break;
            }
        }
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
    currentPositionIndex = 0;
    motorMoving = false;
    servoMoving = false;
    cylinderOperating = false;
    testDelay = 0;
    cylinderDelay = 0;
    cylinderStep = 0;
    
    //! ************************************************************************
    //! RESET SERVO CONTROLLER STATE
    //! ************************************************************************
    if (testServoController) {
        // Reset servo controller to idle state
        testServoController->stop();
        Serial.println("Test state reset - servo controller stopped");
    }
}

void setTestReferences(FastAccelStepper* motor, ServoAccelerationController* servoController, CylinderControl* cylinder) {
    //! ************************************************************************
    //! SET REFERENCES TO MOTOR, SERVO CONTROLLER, AND CYLINDER OBJECTS
    //! ************************************************************************
    testZMotor = motor;
    testServoController = servoController;
    testCylinder = cylinder;
    Serial.println("Test references set - Motor: " + String(motor ? "VALID" : "NULL") + 
                   ", ServoController: " + String(servoController ? "VALID" : "NULL") +
                   ", Cylinder: " + String(cylinder ? "VALID" : "NULL"));
} 