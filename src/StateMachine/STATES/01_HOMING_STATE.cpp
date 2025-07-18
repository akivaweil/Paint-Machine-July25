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
static const float SERVO_MOVEMENT_ANGLE = 30.0; // 30 degrees in each direction (doubled)

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
        
        //! ************************************************************************
        //! STEP 2: START MOVING TOWARD HOME
        //! ************************************************************************
        if (homingZMotor) {
            homingZMotor->runBackward();
            Serial.println("Moving toward home switch...");
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
        // Update the home switch state
        homingZHomeSwitch->update();
        
        // Debug: Print switch state
        static bool lastSwitchState = false;
        bool currentSwitchState = homingZHomeSwitch->read();
        if (currentSwitchState != lastSwitchState) {
            Serial.println("Home switch state changed to: " + String(currentSwitchState ? "TRIGGERED" : "NOT TRIGGERED"));
            lastSwitchState = currentSwitchState;
        }
        
        // Check if home switch is triggered (active high)
        if (currentSwitchState) {
            //! ************************************************************************
            //! STEP 4: HOME SWITCH TRIGGERED - STOP AND SET HOME
            //! ************************************************************************
            homingZMotor->forceStop();
            homingZMotor->setCurrentPosition(0); // Set current position as home (0)
            
            Serial.println("Home switch triggered - Z-axis homed");
            Serial.println("Current position set to 0");
            
            // Start servo sequence with acceleration curves
            if (servoController && homingServo) {
                Serial.println("Servo references are valid - starting sequence");
                
                // Test direct servo movement first
                Serial.println("Testing direct servo movement in homing state...");
                homingServo->write(0);
                delay(1000);
                homingServo->write(180);
                delay(1000);
                homingServo->write(90);
                delay(1000);
                Serial.println("Direct servo test in homing state complete");
                
                servoSequenceStarted = true;
                currentServoMovement = 0;
                
                // Set initial servo position to center (90 degrees)
                servoController->setCurrentAngle(90.0);
                homingServo->write(90.0);
                
                Serial.println("Starting servo acceleration sequence...");
                
                // Start the first movement immediately
                float baseAccel = 0.01; // Base acceleration rate (10x faster)
                float currentAccel = baseAccel * (currentServoMovement + 1); // First movement acceleration
                
                // Set acceleration profile for first movement
                servoController->setAccelerationProfile(
                    currentAccel,  // Acceleration rate
                    currentAccel,  // Deceleration rate (same as acceleration)
                    currentAccel * 1000  // Max velocity (calculated from acceleration)
                );
                
                // Start first movement to 105 degrees
                servoController->moveTo(90.0 + SERVO_MOVEMENT_ANGLE);
                
                Serial.println("Servo movement 1 - Moving to 105 degrees with acceleration " + String(currentAccel));
            } else {
                homingComplete = true;
            }
        }
    }
    
    //! ************************************************************************
    //! STEP 5: EXECUTE SERVO ACCELERATION SEQUENCE
    //! ************************************************************************
    if (servoSequenceStarted && servoController && homingServo) {
        // Debug: Print servo status
        static unsigned long lastDebugTime = 0;
        if (millis() - lastDebugTime > 1000) { // Print every second
            Serial.println("Servo sequence active - Movement: " + String(currentServoMovement) + 
                          ", Current angle: " + String(servoController->getCurrentAngle()) + 
                          ", Target reached: " + String(servoController->hasReachedTarget()));
            lastDebugTime = millis();
        }
        
        // Update servo controller
        servoController->update();
        
        // Check if current movement is complete
        if (servoController->hasReachedTarget()) {
            currentServoMovement++;
            
            if (currentServoMovement < TOTAL_SERVO_MOVEMENTS) {
                // Calculate acceleration for this movement (increasing with each movement)
                float baseAccel = 0.01; // Base acceleration rate (10x faster)
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
    
    // Only show error if we don't have the required components
    if (!homingZMotor || !homingZHomeSwitch) {
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
    
    Serial.println("Homing motor/switch references set - Motor: " + String(motor ? "VALID" : "NULL") + 
                  ", Switch: " + String(homeSwitch ? "VALID" : "NULL"));
}

void setHomingServoReferences(ServoControl* servo, ServoAccelerationController* controller) {
    //! ************************************************************************
    //! SET REFERENCES TO SERVO AND ACCELERATION CONTROLLER
    //! ************************************************************************
    homingServo = servo;
    servoController = controller;
    
    Serial.println("Homing servo references set - Servo: " + String(servo ? "VALID" : "NULL") + 
                  ", Controller: " + String(controller ? "VALID" : "NULL"));
} 