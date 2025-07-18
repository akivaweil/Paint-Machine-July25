//* ************************************************************************
//* ************************ HOMING STATE *********************************
//* ************************************************************************
//! HOMING state - Performs homing sequence for Z-axis
//! Moves toward home switch until triggered, then sets position to 0

#include <Arduino.h>
#include "StateMachine.h"
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"
#include <FastAccelStepper.h>
#include <Bounce2.h>
#include "ServoAccelerationController.h"
#include "ServoControl.h"

//* ************************************************************************
//* ************************ HOMING STATE VARIABLES ***********************
//* ************************************************************************
static bool homingStateInitialized = false;
static bool homingComplete = false;
static FastAccelStepper* homingZMotor = NULL;
static Bounce2::Button* homingZHomeSwitch = NULL;

// Servo acceleration control variables
static ServoControl* homingServo = NULL;
static ServoAccelerationController* servoController = NULL;
static bool servoSequenceStarted = false;
static int currentServoMovement = 0;
static const int TOTAL_SERVO_MOVEMENTS = 5;
static const float SERVO_MOVEMENT_ANGLE = 15.0; // 15 degrees in each direction

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
        
        //! ************************************************************************
        //! STEP 1: SET HOMING SPEED
        //! ************************************************************************
        if (homingZMotor) {
            homingZMotor->setSpeedInHz(Z_HOMING_SPEED);
            Serial.println("Z Motor homing speed set to: " + String(Z_HOMING_SPEED) + " Hz");
        }
    }
    
    //! ************************************************************************
    //! STEP 2: CHECK IF HOMING IS COMPLETE
    //! ************************************************************************
    if (homingComplete) {
        Serial.println("Homing sequence complete - transitioning to IDLE");
        setState(IDLE_STATE);
        return;
    }
    
    //! ************************************************************************
    //! STEP 3: PERFORM HOMING SEQUENCE
    //! ************************************************************************
    if (homingZMotor && homingZHomeSwitch) {
        // Start moving in negative direction (toward home)
        if (!homingZMotor->isRunning()) {
            homingZMotor->runBackward();
            Serial.println("Moving toward home switch...");
        }
        
        // Check if home switch is triggered (active high)
        if (homingZHomeSwitch->read()) {
                    //! ************************************************************************
        //! STEP 4: HOME SWITCH TRIGGERED - STOP AND SET HOME
        //! ************************************************************************
        homingZMotor->forceStop();
        homingZMotor->setCurrentPosition(0); // Set current position as home (0)
        
        Serial.println("Home switch triggered - Z-axis homed");
        Serial.println("Current position set to 0");
        
        // Start servo sequence with acceleration curves
        if (servoController && homingServo) {
            servoSequenceStarted = true;
            currentServoMovement = 0;
            
            // Set initial servo position to center (90 degrees)
            servoController->setCurrentAngle(90.0);
            homingServo->write(90.0);
            
            Serial.println("Starting servo acceleration sequence...");
        } else {
            homingComplete = true;
        }
        }
    }
    
    //! ************************************************************************
    //! STEP 5: EXECUTE SERVO ACCELERATION SEQUENCE
    //! ************************************************************************
    if (servoSequenceStarted && servoController && homingServo) {
        // Update servo controller
        servoController->update();
        
        // Check if current movement is complete
        if (servoController->hasReachedTarget()) {
            currentServoMovement++;
            
            if (currentServoMovement < TOTAL_SERVO_MOVEMENTS) {
                // Calculate acceleration for this movement (increasing with each movement)
                float baseAccel = 0.001; // Base acceleration rate
                float currentAccel = baseAccel * (currentServoMovement + 1); // Increase acceleration each time
                
                // Set acceleration profile for this movement
                servoController->setAccelerationProfile(
                    currentAccel,  // Acceleration rate
                    currentAccel,  // Deceleration rate (same as acceleration)
                    currentAccel * 1000  // Max velocity (calculated from acceleration)
                );
                
                // Calculate target angle (alternate between +15 and -15 degrees from center)
                float targetAngle;
                if (currentServoMovement % 2 == 0) {
                    targetAngle = 90.0 + SERVO_MOVEMENT_ANGLE; // Move to 105 degrees
                } else {
                    targetAngle = 90.0 - SERVO_MOVEMENT_ANGLE; // Move to 75 degrees
                }
                
                // Start the movement
                servoController->moveTo(targetAngle);
                
                Serial.println("Servo movement " + String(currentServoMovement) + 
                              " - Moving to " + String(targetAngle) + 
                              " degrees with acceleration " + String(currentAccel));
            } else {
                // All movements complete
                Serial.println("Servo acceleration sequence complete");
                servoSequenceStarted = false;
                homingComplete = true;
            }
        }
    }
    
    if (!servoSequenceStarted && !homingComplete) {
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
    servoSequenceStarted = false;
    currentServoMovement = 0;
}

void setHomingReferences(FastAccelStepper* motor, Bounce2::Button* homeSwitch) {
    //! ************************************************************************
    //! SET REFERENCES TO MOTOR AND SWITCH OBJECTS
    //! ************************************************************************
    homingZMotor = motor;
    homingZHomeSwitch = homeSwitch;
}

void setHomingServoReferences(ServoControl* servo, ServoAccelerationController* controller) {
    //! ************************************************************************
    //! SET REFERENCES TO SERVO AND ACCELERATION CONTROLLER
    //! ************************************************************************
    homingServo = servo;
    servoController = controller;
} 