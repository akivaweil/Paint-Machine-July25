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
    int angle_degrees;       // Servo angle in degrees
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
    {2.40, 27, "Position 1 - High"},      // Position 1: 25 inches, 0 degrees
    {12.0, 45, "Position 2 - Mid-High"}, // Position 2: 15 inches, 45 degrees
    {20.0, 90, "Position 3 - Center"},   // Position 3: 10 inches, 90 degrees
    {5.0, 135, "Position 4 - Mid-Low"},  // Position 4: 5 inches, 135 degrees
    {1.0, 180, "Position 5 - Low"}       // Position 5: 1 inch, 180 degrees
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
static int cylinderStep = 0;  // Current step in cylinder operation sequence

// Manual command mode variables
static bool manualMode = false;       // Flag to enable manual command mode
static bool manualMotorMoving = false; // Track manual motor movement
static bool manualServoMoving = false; // Track manual servo movement
static String inputBuffer = "";       // Buffer for incoming serial commands

// Servo completion tracking - using calculated completion time
static unsigned long servoMoveStartTime = 0;  // When servo movement started

// Stepper motor test settings
static const float Z_TEST_MAX_SPEED = 20000;     // Stepper max speed for test (steps/sec)
static const float Z_TEST_ACCELERATION = 15000;   // Stepper acceleration for test (steps/sec^2)

// Cylinder sequence settings
static const unsigned long CYLINDER_WAIT_TIME = 1000;  // 1 second wait time
static const float CYLINDER_MOVE_DISTANCE = 0.5;       // 0.5 inches up movement
static const float CYLINDER_MOVE_SPEED = 5000;         // Stepper speed for cylinder up movement (steps/sec)
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
        Serial.println("Type 'mode' to switch to manual command mode");
        Serial.println("Manual commands: z<height>, a<angle>, help, status");
        testStateInitialized = true;
        testComplete = false;
        currentPositionIndex = 0;
        motorMoving = false;
        servoMoving = false;
        testDelay = 0;
        manualMode = false; // Start in auto test mode
        manualMotorMoving = false;
        manualServoMoving = false;
        inputBuffer = "";
        
        //! ************************************************************************
        //! STEP 1: START TEST SEQUENCE
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
    //! STEP 3: CHECK FOR SERIAL COMMANDS
    //! ************************************************************************
    checkSerialCommands();
    
    //! ************************************************************************
    //! STEP 4: HANDLE MANUAL MODE
    //! ************************************************************************
    if (manualMode) {
        // Update manual movement status
        if (testZMotor && manualMotorMoving) {
            if (!testZMotor->isRunning()) {
                manualMotorMoving = false;
                Serial.println("Manual Z movement complete");
            }
        }
        
        if (testServoController && manualServoMoving) {
            if (testServoController->isMoveComplete()) {
                manualServoMoving = false;
                Serial.println("Manual servo movement complete");
            }
        }
        
        return; // Skip auto test sequence in manual mode
    }
    
    //! ************************************************************************
    //! STEP 5: EXECUTE AUTO TEST SEQUENCE
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
            int currentPos = testZMotor->getCurrentPosition();
            Serial.println("Motor stopped at position: " + String(currentPos) + " steps, target was: " + String(Z_HOME_OFFSET_STEPS) + " steps");
            
            // Simplified check - just mark as complete when motor stops
            Serial.println("Test sequence complete - motor has stopped");
            testComplete = true;
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
                testServoController->moveTo(currentPos.angle_degrees);
                servoMoving = true;
                servoMoveStartTime = millis();  // Record when servo movement started
                Serial.println("Servo moving to: " + String(currentPos.angle_degrees) + " degrees");
            }
        }
        
        // Check if motor movement is complete and wait for servo time
        if (testZMotor) {
            bool motorComplete = !testZMotor->isRunning();
            
            // Check if servo has reached target using calculated completion time
            if (motorComplete && servoMoving && testServoController) {
                if (testServoController->isMoveComplete()) {
                    servoMoving = false;
                    Serial.println("Servo reached target position");
                }
            }
            
            // Debug output
            static unsigned long lastDebugTime = 0;
            if (millis() - lastDebugTime > 1000) { // Print every second
                TestPosition currentPos = TEST_POSITIONS[currentPositionIndex];
                unsigned long remainingTime = testServoController ? testServoController->getRemainingMoveTime() : 0;
                
                Serial.println("Debug Position " + String(currentPositionIndex + 1) + " - Motor running: " + String(motorComplete ? "NO" : "YES") + 
                               ", Servo moving: " + String(servoMoving ? "YES" : "NO") +
                               ", Remaining time: " + String(remainingTime) + "ms");
                lastDebugTime = millis();
            }
            
            // Move to next position when motor is complete AND servo move is complete
            if (motorComplete && !servoMoving) {
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
        //! STEP 5A: EXECUTE CYLINDER SEQUENCE
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
    //! STEP 6: CHECK IF TEST IS COMPLETE
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
    
    // Reset manual mode variables
    manualMode = false;
    manualMotorMoving = false;
    manualServoMoving = false;
    inputBuffer = "";
}

void setTestReferences(FastAccelStepper* motor, ServoAccelerationController* servoController, CylinderControl* cylinder) {
    //! ************************************************************************
    //! SET REFERENCES TO MOTOR, SERVO CONTROLLER, AND CYLINDER OBJECTS
    //! ************************************************************************
    testZMotor = motor;
    testServoController = servoController;
    testCylinder = cylinder;
    Serial.println("Test references set - Motor: " + String(motor ? "VALID" : "NULL") + 
                   ", Servo Controller: " + String(servoController ? "VALID" : "NULL") +
                   ", Cylinder: " + String(cylinder ? "VALID" : "NULL"));
} 

//! ************************************************************************
//! COMMAND PARSING FUNCTIONS FOR MANUAL MODE
//! ************************************************************************

void parseManualCommand(String command) {
    //! ************************************************************************
    //! PARSE MANUAL COMMANDS: z1.3, a30, etc.
    //! ************************************************************************
    command.trim();
    command.toLowerCase();
    
    if (command.length() < 2) {
        Serial.println("Invalid command format. Use: z<height> or a<angle>");
        Serial.println("Examples: z1.3, a30, z5.0, a90");
        return;
    }
    
    char commandType = command.charAt(0);
    String valueStr = command.substring(1);
    float value = valueStr.toFloat();
    
    switch (commandType) {
        case 'z': // Z-axis height command
            if (value > 0 && value <= 50) { // Reasonable height limits
                Serial.println("Manual Z command: Moving to " + String(value) + " inches");
                moveToManualHeight(value);
            } else {
                Serial.println("Invalid Z height. Must be between 0.1 and 50 inches");
            }
            break;
            
        case 'a': // Servo angle command
            if (value >= 0 && value <= 180) { // Servo angle limits
                Serial.println("Manual angle command: Moving servo to " + String(value) + " degrees");
                moveToManualAngle(value);
            } else {
                Serial.println("Invalid angle. Must be between 0 and 180 degrees");
            }
            break;
            
        case 'h': // Help command
            if (command == "help") {
                printManualModeHelp();
            }
            break;
            
        case 's': // Status command
            if (command == "status") {
                printManualModeStatus();
            }
            break;
            
        case 'm': // Mode toggle
            if (command == "mode") {
                toggleManualMode();
            }
            break;
            
        default:
            Serial.println("Unknown command: " + command);
            Serial.println("Use: z<height>, a<angle>, help, status, mode");
            break;
    }
}

void moveToManualHeight(float heightInches) {
    //! ************************************************************************
    //! MOVE Z-AXIS TO SPECIFIED HEIGHT IN MANUAL MODE
    //! ************************************************************************
    if (!testZMotor) {
        Serial.println("ERROR: Z motor not available");
        return;
    }
    
    if (manualMotorMoving) {
        Serial.println("Motor already moving - command ignored");
        return;
    }
    
    int targetSteps = (int)(heightInches * STEPS_PER_INCH);
    testZMotor->setSpeedInHz(Z_TEST_MAX_SPEED);
    testZMotor->setAcceleration(Z_TEST_ACCELERATION);
    testZMotor->moveTo(targetSteps);
    manualMotorMoving = true;
    
    Serial.println("Moving Z to: " + String(targetSteps) + " steps (" + String(heightInches) + " inches)");
    Serial.println("Speed: " + String(Z_TEST_MAX_SPEED) + " steps/sec, Accel: " + String(Z_TEST_ACCELERATION) + " steps/sec²");
}

void moveToManualAngle(int angleDegrees) {
    //! ************************************************************************
    //! MOVE SERVO TO SPECIFIED ANGLE IN MANUAL MODE
    //! ************************************************************************
    if (!testServoController) {
        Serial.println("ERROR: Servo not available");
        return;
    }
    
    if (manualServoMoving) {
        Serial.println("Servo already moving - command ignored");
        return;
    }
    
    testServoController->moveTo(angleDegrees);
    manualServoMoving = true;
    servoMoveStartTime = millis();  // Record when servo movement started
    
    Serial.println("Moving servo to: " + String(angleDegrees) + " degrees");
}

void printManualModeHelp() {
    //! ************************************************************************
    //! PRINT MANUAL MODE HELP INFORMATION
    //! ************************************************************************
    Serial.println("=== MANUAL MODE COMMANDS ===");
    Serial.println("z<height>  - Move Z-axis to height (inches)");
    Serial.println("           Example: z1.3, z5.0, z10.5");
    Serial.println("a<angle>   - Move servo to angle (degrees)");
    Serial.println("           Example: a30, a90, a135");
    Serial.println("help       - Show this help message");
    Serial.println("status     - Show current positions and status");
    Serial.println("mode       - Toggle between manual and auto test mode");
    Serial.println("");
    Serial.println("Current mode: " + String(manualMode ? "MANUAL" : "AUTO TEST"));
}

void printManualModeStatus() {
    //! ************************************************************************
    //! PRINT CURRENT STATUS IN MANUAL MODE
    //! ************************************************************************
    Serial.println("=== MANUAL MODE STATUS ===");
    
    if (testZMotor) {
        int currentSteps = testZMotor->getCurrentPosition();
        float currentInches = (float)currentSteps / STEPS_PER_INCH;
        Serial.println("Z Position: " + String(currentSteps) + " steps (" + String(currentInches, 2) + " inches)");
        Serial.println("Z Motor running: " + String(testZMotor->isRunning() ? "YES" : "NO"));
    } else {
        Serial.println("Z Motor: NOT AVAILABLE");
    }
    
    if (testServoController) {
        Serial.println("Servo: AVAILABLE");
    } else {
        Serial.println("Servo: NOT AVAILABLE");
    }
    
    Serial.println("Mode: " + String(manualMode ? "MANUAL" : "AUTO TEST"));
}

void toggleManualMode() {
    //! ************************************************************************
    //! TOGGLE BETWEEN MANUAL AND AUTO TEST MODE
    //! ************************************************************************
    manualMode = !manualMode;
    
    if (manualMode) {
        Serial.println("=== SWITCHING TO MANUAL MODE ===");
        Serial.println("Type commands like: z1.3, a30, help, status");
        Serial.println("Use 'mode' to switch back to auto test");
    } else {
        Serial.println("=== SWITCHING TO AUTO TEST MODE ===");
        Serial.println("Auto test sequence will continue");
    }
}

void checkSerialCommands() {
    //! ************************************************************************
    //! CHECK FOR INCOMING SERIAL COMMANDS IN MANUAL MODE
    //! ************************************************************************
    if (!manualMode) {
        return; // Only process commands in manual mode
    }
    
    while (Serial.available()) {
        char c = Serial.read();
        
        if (c == '\n' || c == '\r') {
            if (inputBuffer.length() > 0) {
                parseManualCommand(inputBuffer);
                inputBuffer = "";
            }
        } else {
            inputBuffer += c;
        }
    }
} 