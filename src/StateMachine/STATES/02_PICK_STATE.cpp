//* ************************************************************************
//* ************************ PICK STATE ********************************
//* ************************************************************************
//! PICK state - Performs picking operations with cylinder control
//! This state handles the picking sequence: move to position, extend cylinder, adjust height, retract

#include <Arduino.h>
#include "StateMachine.h"
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"
#include "ServoAccelerationController.h"
#include "LoaderForkStepper.h"
#include <FastAccelStepper.h>

//* ************************************************************************
//* ************************ PICK STATE VARIABLES ********************
//* ************************************************************************
static bool pickStateInitialized = false;
static bool pickComplete = false;
static ServoAccelerationController* pickServoController = NULL;
static LoaderForkStepper* pickLoaderFork = NULL;
static FastAccelStepper* pickZMotor = NULL;
static int currentStep = 0;
static unsigned long stepStartTime = 0;
static int targetHeightSteps = 0;

//* ************************************************************************
//* ************************ PICK STATE FUNCTIONS ********************
//* ************************************************************************

void executePickState() {
    //! ************************************************************************
    //! EXECUTE PICK STATE - PICKING OPERATION WITH CYLINDER CONTROL
    //! ************************************************************************
    
    // Initialize pick state on first entry
    if (!pickStateInitialized) {
        Serial.println("=== ENTERING PICK STATE ===");
        Serial.println("Starting picking operation...");
        pickStateInitialized = true;
        pickComplete = false;
        currentStep = 0;
        stepStartTime = millis();
        
        //! ************************************************************************
        //! STEP 0: SETUP INITIAL PARAMETERS
        //! ************************************************************************
        targetHeightSteps = PICK_HEIGHT_STEPS;
        
        //! ************************************************************************
        //! SLOT CONFIGURATION USAGE EXAMPLE:
        //! ************************************************************************
        // To use a specific slot instead of fixed positions:
        // int slotNumber = 5; // Change this to use different slots
        // SlotPosition slotPos = getSlotPosition(slotNumber);
        // targetHeightSteps = getSlotHeightSteps(slotNumber);
        // Then use slotPos.servo_angle instead of RETRIEVE_ANGLE_DEGREES
        
        // Set motor speed and acceleration
        if (pickZMotor) {
            pickZMotor->setSpeedInHz(Z_MAX_SPEED);
            pickZMotor->setAcceleration(Z_ACCELERATION);
        }
        
        // Set servo acceleration profile
        if (pickServoController) {
            pickServoController->setAccelerationProfile(300, 2000);
        }
    }
    
    //! ************************************************************************
    //! STEP 1: CHECK IF PICKING IS COMPLETE
    //! ************************************************************************
    if (pickComplete) {
        Serial.println("Picking operation complete - transitioning to PLACE state");
        setState(PLACE_STATE);
        return;
    }
    
    //! ************************************************************************
    //! STEP 2: PERFORM PICKING SEQUENCE
    //! ************************************************************************
    unsigned long currentTime = millis();
    
    switch (currentStep) {
        case 0: {
            //! ************************************************************************
            //! STEP 0: MOVE TO SPECIFIC HEIGHT AND ANGLE
            //! ************************************************************************
            if (currentTime - stepStartTime >= 100) { // Small delay to ensure initialization
                Serial.println("Step 0: Moving to pick position - Height: " + String(PICK_HEIGHT_INCHES) + " inches, Angle: " + String(PICK_ANGLE_DEGREES) + " degrees");
                
                // Move Z motor to pick height
                if (pickZMotor) {
                    pickZMotor->moveTo(targetHeightSteps);
                }
                
                // Move servo to pick angle
                if (pickServoController) {
                    pickServoController->moveTo(PICK_ANGLE_DEGREES);
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
            bool zMotorReady = !pickZMotor || !pickZMotor->isRunning();
            bool servoReady = !pickServoController || pickServoController->hasReachedTarget();
            
            if (zMotorReady && servoReady) {
                Serial.println("Step 1: Position reached, extending cylinder");
                currentStep = 2;
                stepStartTime = currentTime;
            }
            break;
        }
            
        case 2: {
            //! ************************************************************************
            //! STEP 2: EXTEND THE LOADER FORK
            //! ************************************************************************
            if (pickLoaderFork) {
                pickLoaderFork->extend();
                Serial.println("Step 2: Loader fork extended");
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
            //! STEP 4: RAISE HEIGHT BY .7 INCHES
            //! ************************************************************************
            if (pickZMotor) {
                int newHeightSteps = targetHeightSteps + HEIGHT_ADJUSTMENT_STEPS;
                pickZMotor->moveTo(newHeightSteps);
                Serial.println("Step 4: Raising height to " + String((float)newHeightSteps / STEPS_PER_INCH) + " inches");
            }
            currentStep = 5;
            stepStartTime = currentTime;
            break;
        }
            
        case 5: {
            //! ************************************************************************
            //! STEP 5: WAIT FOR HEIGHT ADJUSTMENT TO COMPLETE
            //! ************************************************************************
            bool heightAdjustmentComplete = !pickZMotor || !pickZMotor->isRunning();
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
            if (pickLoaderFork) {
                pickLoaderFork->retract();
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
                Serial.println("Step 8: Loader fork retract wait complete, picking sequence finished");
                pickComplete = true;
            }
            break;
        }
            
        default: {
            Serial.println("ERROR: Invalid step in picking sequence");
            pickComplete = true;
            break;
        }
    }
}

void resetPickState() {
    //! ************************************************************************
    //! RESET PICK STATE FLAGS
    //! ************************************************************************
    pickStateInitialized = false;
    pickComplete = false;
    currentStep = 0;
    stepStartTime = 0;
    targetHeightSteps = 0;
}

void setPickReferences(ServoAccelerationController* servoController, LoaderForkStepper* loaderFork, FastAccelStepper* zMotor) {
    //! ************************************************************************
    //! SET REFERENCES TO SERVO CONTROLLER, LOADER FORK, AND Z MOTOR OBJECTS
    //! ************************************************************************
    pickServoController = servoController;
    pickLoaderFork = loaderFork;
    pickZMotor = zMotor;
} 