//* ************************************************************************
//* ************************ IDLE STATE ***********************************
//* ************************************************************************
//! IDLE state - Machine waits for commands
//! This is the default state when no operations are running

#include <Arduino.h>
#include "StateMachine.h"
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"
#include "ServoControl.h"
#include "ServoAccelerationController.h"
#include <FastAccelStepper.h>

//* ************************************************************************
//* ************************ IDLE STATE VARIABLES *************************
//* ************************************************************************
static bool idleStateInitialized = false;
static ServoControl* idleServo = NULL;
static ServoAccelerationController* idleServoController = NULL;
static FastAccelStepper* idleZMotor = NULL;
static bool idlePositionReached = false;
static bool servoMoveComplete = false;

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
        Serial.println("Moving to loading tray position - Height: 0.3 inches, Angle: 48.0 degrees");
        
        // Set motor speed and acceleration
        if (idleZMotor) {
            idleZMotor->setSpeedInHz(Z_MAX_SPEED);
            idleZMotor->setAcceleration(Z_ACCELERATION);
            // Move to loading tray height (0.3 inches)
            int loadingTraySteps = (int)(0.3 * STEPS_PER_INCH);
            idleZMotor->moveTo(loadingTraySteps);
        }
        
        // Set servo to loading tray position using regular ServoControl (not acceleration controller)
        if (idleServo) {
            idleServo->write(48.0);
            Serial.println("Setting servo to loading tray position (48.0 degrees) using regular ServoControl");
            servoMoveComplete = false;
        }
        
        idleStateInitialized = true;
        idlePositionReached = false;
    }
    
    //! ************************************************************************
    //! STEP 1: CHECK IF IDLE POSITION IS REACHED
    //! ************************************************************************
    if (!idlePositionReached) {
        bool zMotorReady = !idleZMotor || !idleZMotor->isRunning();
        bool servoReady = !idleServo || servoMoveComplete || idleServo->hasReachedTarget();
        
        // Check if servo movement is complete
        if (idleServo && !servoMoveComplete) {
            if (idleServo->hasReachedTarget()) {
                servoMoveComplete = true;
                Serial.println("Servo reached idle position");
            }
        }
        
        if (zMotorReady && servoReady) {
            Serial.println("Loading tray position reached - Machine ready for commands");
            idlePositionReached = true;
        }
    }
    
    //! ************************************************************************
    //! STEP 2: UPDATE SERVO CONTROLLER (for future operations)
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
    servoMoveComplete = false;
}

void setIdleReferences(ServoControl* servo, ServoAccelerationController* servoController, FastAccelStepper* zMotor) {
    //! ************************************************************************
    //! SET REFERENCES TO SERVO OBJECTS AND Z MOTOR OBJECTS
    //! ************************************************************************
    idleServo = servo;
    idleServoController = servoController;
    idleZMotor = zMotor;
    Serial.println("Idle references set - Servo: " + String(servo ? "VALID" : "NULL") + 
                  ", Servo Controller: " + String(servoController ? "VALID" : "NULL") + 
                  ", Z Motor: " + String(zMotor ? "VALID" : "NULL"));
} 