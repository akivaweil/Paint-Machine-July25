//* ************************************************************************
//* ************************ RETRIEVE STATE ******************************
//* ************************************************************************
//! RETRIEVE state - Performs retrieving operations with cylinder control
//! This state handles the retrieval sequence: move to position, extend cylinder, adjust height, retract

#include <Arduino.h>
#include "StateMachine.h"
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"
#include "ServoAccelerationController.h"
#include "CylinderControl.h"
#include <FastAccelStepper.h>

//* ************************************************************************
//* ************************ RETRIEVE STATE VARIABLES ********************
//* ************************************************************************
static bool retrieveStateInitialized = false;
static bool retrieveComplete = false;
static ServoAccelerationController* retrieveServoController = NULL;
static CylinderControl* retrieveCylinder = NULL;
static FastAccelStepper* retrieveZMotor = NULL;
static int currentStep = 0;
static unsigned long stepStartTime = 0;
static int targetHeightSteps = 0;

//* ************************************************************************
//* ************************ RETRIEVE STATE FUNCTIONS ********************
//* ************************************************************************

void executeRetrieveState() {
    //! ************************************************************************
    //! EXECUTE RETRIEVE STATE - RETRIEVAL OPERATION WITH CYLINDER CONTROL
    //! ************************************************************************
    
    // Initialize retrieve state on first entry
    if (!retrieveStateInitialized) {
        Serial.println("=== ENTERING RETRIEVE STATE ===");
        Serial.println("Starting retrieval operation...");
        retrieveStateInitialized = true;
        retrieveComplete = false;
        currentStep = 0;
        stepStartTime = millis();
        
        //! ************************************************************************
        //! STEP 0: SETUP INITIAL PARAMETERS
        //! ************************************************************************
        targetHeightSteps = RETRIEVE_HEIGHT_STEPS;
        
        // Set motor speed and acceleration
        if (retrieveZMotor) {
            retrieveZMotor->setSpeedInHz(Z_MAX_SPEED);
            retrieveZMotor->setAcceleration(Z_ACCELERATION);
        }
        
        // Set servo acceleration profile
        if (retrieveServoController) {
            retrieveServoController->setAccelerationProfile(300, 2000);
        }
    }
    
    //! ************************************************************************
    //! STEP 1: CHECK IF RETRIEVING IS COMPLETE
    //! ************************************************************************
    if (retrieveComplete) {
        Serial.println("Retrieval operation complete - transitioning to STORE state");
        setState(STORE_STATE);
        return;
    }
    
    //! ************************************************************************
    //! STEP 2: PERFORM RETRIEVAL SEQUENCE
    //! ************************************************************************
    unsigned long currentTime = millis();
    
    switch (currentStep) {
        case 0: {
            //! ************************************************************************
            //! STEP 0: MOVE TO SPECIFIC HEIGHT AND ANGLE
            //! ************************************************************************
            if (currentTime - stepStartTime >= 100) { // Small delay to ensure initialization
                Serial.println("Step 0: Moving to retrieve position - Height: " + String(RETRIEVE_HEIGHT_INCHES) + " inches, Angle: " + String(RETRIEVE_ANGLE_DEGREES) + " degrees");
                
                // Move Z motor to retrieve height
                if (retrieveZMotor) {
                    retrieveZMotor->moveTo(targetHeightSteps);
                }
                
                // Move servo to retrieve angle
                if (retrieveServoController) {
                    retrieveServoController->moveTo(RETRIEVE_ANGLE_DEGREES);
                }
                
                currentStep = 1;
                stepStartTime = currentTime;
            }
            break;
        }
            
        case 1: {
            //! ************************************************************************
            //! STEP 1: WAIT FOR MOTOR AND SERVO TO REACH POSITION
            //! ************************************************************************
            bool zMotorReady = !retrieveZMotor || !retrieveZMotor->isRunning();
            bool servoReady = !retrieveServoController || retrieveServoController->hasReachedTarget();
            
            if (zMotorReady && servoReady) {
                Serial.println("Step 1: Position reached, extending cylinder");
                currentStep = 2;
                stepStartTime = currentTime;
            }
            break;
        }
            
        case 2: {
            //! ************************************************************************
            //! STEP 2: EXTEND THE CYLINDER
            //! ************************************************************************
            if (retrieveCylinder) {
                retrieveCylinder->extend();
                Serial.println("Step 2: Cylinder extended");
            }
            currentStep = 3;
            stepStartTime = currentTime;
            break;
        }
            
        case 3: {
            //! ************************************************************************
            //! STEP 3: WAIT 750MS AFTER EXTENDING CYLINDER
            //! ************************************************************************
            if (currentTime - stepStartTime >= CYLINDER_EXTEND_WAIT) {
                Serial.println("Step 3: Wait complete, raising height by " + String(HEIGHT_ADJUSTMENT_INCHES) + " inches");
                currentStep = 4;
                stepStartTime = currentTime;
            }
            break;
        }
            
        case 4: {
            //! ************************************************************************
            //! STEP 4: RAISE HEIGHT BY .4 INCHES
            //! ************************************************************************
            if (retrieveZMotor) {
                int newHeightSteps = targetHeightSteps + HEIGHT_ADJUSTMENT_STEPS;
                retrieveZMotor->moveTo(newHeightSteps);
                Serial.println("Step 4: Raising height to " + String((float)newHeightSteps / STEPS_PER_INCH) + " inches");
            }
            currentStep = 5;
            stepStartTime = currentTime;
            break;
        }
            
        case 5: {
            //! ************************************************************************
            //! STEP 5: WAIT 100MS AFTER HEIGHT ADJUSTMENT
            //! ************************************************************************
            if (currentTime - stepStartTime >= HEIGHT_ADJUST_WAIT) {
                Serial.println("Step 5: Height adjustment wait complete, retracting cylinder");
                currentStep = 6;
                stepStartTime = currentTime;
            }
            break;
        }
            
        case 6: {
            //! ************************************************************************
            //! STEP 6: RETRACT THE CYLINDER
            //! ************************************************************************
            if (retrieveCylinder) {
                retrieveCylinder->retract();
                Serial.println("Step 6: Cylinder retracted");
            }
            currentStep = 7;
            stepStartTime = currentTime;
            break;
        }
            
        case 7: {
            //! ************************************************************************
            //! STEP 7: WAIT 1000MS AFTER RETRACTING CYLINDER
            //! ************************************************************************
            if (currentTime - stepStartTime >= CYLINDER_RETRACT_WAIT) {
                Serial.println("Step 7: Cylinder retract wait complete, starting servo jiggle");
                currentStep = 8;
                stepStartTime = currentTime;
            }
            break;
        }
            
        case 8: {
            //! ************************************************************************
            //! STEP 8: JIGGLE SERVO LEFT (CURRENT ANGLE - JIGGLE ANGLE) AT MAX SPEED
            //! ************************************************************************
            if (retrieveServoController) {
                // Set maximum speed for jiggle movements
                retrieveServoController->setAccelerationProfile(100.0, 180.0); // High acceleration and max speed
                int jiggleLeftAngle = RETRIEVE_ANGLE_DEGREES - SERVO_JIGGLE_ANGLE;
                retrieveServoController->moveTo(jiggleLeftAngle);
                Serial.println("Step 8: Jiggling servo left to " + String(jiggleLeftAngle) + " degrees at max speed");
            }
            currentStep = 9;
            stepStartTime = currentTime;
            break;
        }
            
        case 9: {
            //! ************************************************************************
            //! STEP 9: WAIT FOR SERVO TO REACH LEFT POSITION
            //! ************************************************************************
            bool servoReady = !retrieveServoController || retrieveServoController->hasReachedTarget();
            if (servoReady && (currentTime - stepStartTime >= SERVO_JIGGLE_DELAY)) {
                Serial.println("Step 9: Left jiggle complete, moving to right position");
                currentStep = 10;
                stepStartTime = currentTime;
            }
            break;
        }
            
        case 10: {
            //! ************************************************************************
            //! STEP 10: JIGGLE SERVO RIGHT (CURRENT ANGLE + JIGGLE ANGLE) AT MAX SPEED
            //! ************************************************************************
            if (retrieveServoController) {
                int jiggleRightAngle = RETRIEVE_ANGLE_DEGREES + SERVO_JIGGLE_ANGLE;
                retrieveServoController->moveTo(jiggleRightAngle);
                Serial.println("Step 10: Jiggling servo right to " + String(jiggleRightAngle) + " degrees at max speed");
            }
            currentStep = 11;
            stepStartTime = currentTime;
            break;
        }
            
        case 11: {
            //! ************************************************************************
            //! STEP 11: WAIT FOR SERVO TO REACH RIGHT POSITION
            //! ************************************************************************
            bool servoReady = !retrieveServoController || retrieveServoController->hasReachedTarget();
            if (servoReady && (currentTime - stepStartTime >= SERVO_JIGGLE_DELAY)) {
                Serial.println("Step 11: Right jiggle complete, returning to center position");
                currentStep = 12;
                stepStartTime = currentTime;
            }
            break;
        }
            
        case 12: {
            //! ************************************************************************
            //! STEP 12: RETURN SERVO TO CENTER POSITION AT MAX SPEED
            //! ************************************************************************
            if (retrieveServoController) {
                retrieveServoController->moveTo(RETRIEVE_ANGLE_DEGREES);
                Serial.println("Step 12: Returning servo to center position at " + String(RETRIEVE_ANGLE_DEGREES) + " degrees at max speed");
            }
            currentStep = 13;
            stepStartTime = currentTime;
            break;
        }
            
        case 13: {
            //! ************************************************************************
            //! STEP 13: WAIT FOR SERVO TO REACH CENTER POSITION
            //! ************************************************************************
            bool servoReady = !retrieveServoController || retrieveServoController->hasReachedTarget();
            if (servoReady) {
                Serial.println("Step 13: Servo jiggle complete, retrieval sequence finished");
                retrieveComplete = true;
            }
            break;
        }
            
        default: {
            Serial.println("ERROR: Invalid step in retrieve sequence: " + String(currentStep));
            retrieveComplete = true;
            break;
        }
    }
}

void resetRetrieveState() {
    //! ************************************************************************
    //! RESET RETRIEVE STATE FLAGS
    //! ************************************************************************
    retrieveStateInitialized = false;
    retrieveComplete = false;
    currentStep = 0;
    stepStartTime = 0;
    targetHeightSteps = 0;
}

void setRetrieveReferences(ServoAccelerationController* servoController, CylinderControl* cylinder, FastAccelStepper* zMotor) {
    //! ************************************************************************
    //! SET REFERENCES TO SERVO CONTROLLER, CYLINDER, AND Z MOTOR OBJECTS
    //! ************************************************************************
    retrieveServoController = servoController;
    retrieveCylinder = cylinder;
    retrieveZMotor = zMotor;
} 