//* ************************************************************************
//* ************************ PLACE STATE *********************************
//* ************************************************************************
//! PLACE state - Performs placing operations with cylinder control
//! This state handles the placing sequence: move to position, extend cylinder, adjust height, retract

#include <Arduino.h>
#include "StateMachine.h"
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"
#include "ServoAccelerationController.h"
#include "LoaderForkStepper.h"
#include <FastAccelStepper.h>

//* ************************************************************************
//* ************************ PLACE STATE VARIABLES ***********************
//* ************************************************************************
static bool placeStateInitialized = false;
static bool placeComplete = false;
static ServoAccelerationController* placeServoController = NULL;
static LoaderForkStepper* placeLoaderFork = NULL;
static FastAccelStepper* placeZMotor = NULL;
static int currentStep = 0;
static unsigned long stepStartTime = 0;
static int targetHeightSteps = 0;

//* ************************************************************************
//* ************************ PLACE STATE FUNCTIONS ***********************
//* ************************************************************************

void executePlaceState() {
    //! ************************************************************************
    //! EXECUTE PLACE STATE - PLACING OPERATION WITH CYLINDER CONTROL
    //! ************************************************************************
    
    // Initialize place state on first entry
    if (!placeStateInitialized) {
        Serial.println("=== ENTERING PLACE STATE ===");
        Serial.println("Starting placing operation...");
        placeStateInitialized = true;
        placeComplete = false;
        currentStep = 0;
        stepStartTime = millis();
        
        //! ************************************************************************
        //! STEP 0: SETUP INITIAL PARAMETERS
        //! ************************************************************************
        targetHeightSteps = PLACE_HEIGHT_STEPS;
        
        // Set motor speed and acceleration
        if (placeZMotor) {
            placeZMotor->setSpeedInHz(Z_MAX_SPEED);
            placeZMotor->setAcceleration(Z_ACCELERATION);
        }
        
        // Set servo acceleration profile
        if (placeServoController) {
            placeServoController->setAccelerationProfile(300, 2000);
        }
    }
    
    //! ************************************************************************
    //! STEP 1: CHECK IF PLACING IS COMPLETE
    //! ************************************************************************
    if (placeComplete) {
        Serial.println("Placing operation complete - transitioning to IDLE state");
        setState(IDLE_STATE);
        return;
    }
    
    //! ************************************************************************
    //! STEP 2: PERFORM PLACING SEQUENCE
    //! ************************************************************************
    unsigned long currentTime = millis();
    
    switch (currentStep) {
        case 0: {
            //! ************************************************************************
            //! STEP 0: MOVE TO SPECIFIC HEIGHT AND ANGLE
            //! ************************************************************************
            if (currentTime - stepStartTime >= 100) { // Small delay to ensure initialization
                Serial.println("Step 0: Moving to place position - Height: " + String(PLACE_HEIGHT_INCHES) + " inches, Angle: " + String(PLACE_ANGLE_DEGREES) + " degrees");
                
                // Move Z motor to place height
                if (placeZMotor) {
                    placeZMotor->moveTo(targetHeightSteps);
                }
                
                // Move servo to place angle
                if (placeServoController) {
                    placeServoController->moveTo(PLACE_ANGLE_DEGREES);
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
            bool zMotorReady = !placeZMotor || !placeZMotor->isRunning();
            bool servoReady = !placeServoController || placeServoController->hasReachedTarget();
            
            if (zMotorReady && servoReady) {
                Serial.println("Step 1: Position reached, extending cylinder");
                currentStep = 2;
                stepStartTime = currentTime;
            }
            break;
        }
            
        case 2: {
            //! ************************************************************************
            //! STEP 2: EXTEND THE LOADER FORK TO PLACE POSITION
            //! ************************************************************************
            if (placeLoaderFork) {
                placeLoaderFork->extendToPlacePosition();
                Serial.println("Step 2: Loader fork extending to place position");
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
                Serial.println("Step 3: Wait complete, lowering height by " + String(HEIGHT_ADJUSTMENT_INCHES) + " inches");
                currentStep = 4;
                stepStartTime = currentTime;
            }
            break;
        }
            
        case 4: {
            //! ************************************************************************
            //! STEP 4: LOWER HEIGHT BY .7 INCHES
            //! ************************************************************************
            if (placeZMotor) {
                int newHeightSteps = targetHeightSteps - HEIGHT_ADJUSTMENT_STEPS;
                placeZMotor->moveTo(newHeightSteps);
                Serial.println("Step 4: Lowering height to " + String((float)newHeightSteps / STEPS_PER_INCH) + " inches");
            }
            currentStep = 5;
            stepStartTime = currentTime;
            break;
        }
            
        case 5: {
            //! ************************************************************************
            //! STEP 5: WAIT FOR HEIGHT ADJUSTMENT TO COMPLETE
            //! ************************************************************************
            bool heightAdjustmentComplete = !placeZMotor || !placeZMotor->isRunning();
            if (heightAdjustmentComplete) {
                Serial.println("Step 5: Height adjustment complete, waiting 100ms before retracting cylinder");
                currentStep = 6;
                stepStartTime = currentTime;
            }
            break;
        }
            
        case 6: {
            //! ************************************************************************
            //! STEP 6: WAIT 100MS AFTER HEIGHT ADJUSTMENT
            //! ************************************************************************
            if (currentTime - stepStartTime >= HEIGHT_ADJUST_WAIT) {
                Serial.println("Step 6: Height adjustment wait complete, retracting cylinder");
                currentStep = 7;
                stepStartTime = currentTime;
            }
            break;
        }
            
        case 7: {
            //! ************************************************************************
            //! STEP 7: RETRACT THE LOADER FORK
            //! ************************************************************************
            if (placeLoaderFork) {
                placeLoaderFork->retract();
                Serial.println("Step 7: Loader fork retracted");
            }
            currentStep = 8;
            stepStartTime = currentTime;
            break;
        }
            
        case 8: {
            //! ************************************************************************
            //! STEP 8: WAIT 750MS AFTER RETRACTING LOADER FORK
            //! ************************************************************************
            if (currentTime - stepStartTime >= CYLINDER_RETRACT_WAIT) {
                Serial.println("Step 8: Loader fork retract wait complete, placing sequence finished");
                placeComplete = true;
            }
            break;
        }
            
        default: {
            Serial.println("ERROR: Invalid step in placing sequence");
            placeComplete = true;
            break;
        }
    }
}

void resetPlaceState() {
    //! ************************************************************************
    //! RESET PLACE STATE FLAGS
    //! ************************************************************************
    placeStateInitialized = false;
    placeComplete = false;
    currentStep = 0;
    stepStartTime = 0;
    targetHeightSteps = 0;
}

void setPlaceReferences(ServoAccelerationController* servoController, LoaderForkStepper* loaderFork, FastAccelStepper* zMotor) {
    //! ************************************************************************
    //! SET REFERENCES TO SERVO CONTROLLER, LOADER FORK, AND Z MOTOR OBJECTS
    //! ************************************************************************
    placeServoController = servoController;
    placeLoaderFork = loaderFork;
    placeZMotor = zMotor;
} 