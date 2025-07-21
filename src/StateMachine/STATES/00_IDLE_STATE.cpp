//* ************************************************************************
//* ************************ IDLE STATE ***********************************
//* ************************************************************************
//! IDLE state - Machine waits for commands
//! This is the default state when no operations are running

#include <Arduino.h>
#include "StateMachine.h"
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"
#include "ServoAccelerationController.h"
#include <FastAccelStepper.h>

//* ************************************************************************
//* ************************ IDLE STATE VARIABLES *************************
//* ************************************************************************
static bool idleStateInitialized = false;
static ServoAccelerationController* idleServoController = NULL;
static FastAccelStepper* idleZMotor = NULL;
static bool idlePositionReached = false;

//* ************************************************************************
//* ************************ IDLE STATE FUNCTIONS *************************
//* ************************************************************************

void executeIdleState() {
    //! ************************************************************************
    //! EXECUTE IDLE STATE - WAITING FOR COMMANDS
    //! ************************************************************************
    
    // Initialize idle state on first entry
    if (!idleStateInitialized) {
        Serial.println("=== ENTERING IDLE STATE ===");
        Serial.println("Moving to idle position - Height: " + String(IDLE_HEIGHT_INCHES) + " inches, Angle: " + String(IDLE_ANGLE_DEGREES) + " degrees");
        
        // Set motor speed and acceleration
        if (idleZMotor) {
            idleZMotor->setSpeedInHz(Z_MAX_SPEED);
            idleZMotor->setAcceleration(Z_ACCELERATION);
            idleZMotor->moveTo(IDLE_HEIGHT_STEPS);
        }
        
        // Set servo to idle position (30 degrees)
        if (idleServoController) {
            idleServoController->setAccelerationProfile(250, 2000); // Conservative settings
            idleServoController->moveTo(IDLE_ANGLE_DEGREES);
            Serial.println("Setting servo to idle position (" + String(IDLE_ANGLE_DEGREES) + " degrees)");
        }
        
        idleStateInitialized = true;
        idlePositionReached = false;
    }
    
    //! ************************************************************************
    //! STEP 1: CHECK IF IDLE POSITION IS REACHED
    //! ************************************************************************
    if (!idlePositionReached) {
        bool zMotorReady = !idleZMotor || !idleZMotor->isRunning();
        bool servoReady = !idleServoController || idleServoController->hasReachedTarget();
        
        if (zMotorReady && servoReady) {
            Serial.println("Idle position reached - Machine ready for commands");
            idlePositionReached = true;
        }
    }
    
    //! ************************************************************************
    //! STEP 2: UPDATE SERVO CONTROLLER
    //! ************************************************************************
    if (idleServoController) {
        idleServoController->update();
    }
    
    //! ************************************************************************
    //! STEP 3: PERFORM IDLE TASKS
    //! ************************************************************************
    // Any background tasks can be performed here
    // For now, just maintain the idle state
    
    //! ************************************************************************
    //! STEP 4: STATE TRANSITIONS
    //! ************************************************************************
    // Transitions to other states will be triggered by external events
    // (button presses, commands, etc.) handled in main loop
}

void resetIdleState() {
    //! ************************************************************************
    //! RESET IDLE STATE FLAGS
    //! ************************************************************************
    idleStateInitialized = false;
    idlePositionReached = false;
}

void setIdleReferences(ServoAccelerationController* servoController, FastAccelStepper* zMotor) {
    //! ************************************************************************
    //! SET REFERENCES TO SERVO CONTROLLER AND Z MOTOR OBJECTS
    //! ************************************************************************
    idleServoController = servoController;
    idleZMotor = zMotor;
    Serial.println("Idle references set - Servo Controller: " + String(servoController ? "VALID" : "NULL") + ", Z Motor: " + String(zMotor ? "VALID" : "NULL"));
} 