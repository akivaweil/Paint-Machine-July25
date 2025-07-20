//* ************************************************************************
//* ************************ STORE STATE *********************************
//* ************************************************************************
//! STORE state - Performs storing operations with cylinder control
//! This state handles the storage sequence: move to position, extend cylinder, adjust height, retract

#include <Arduino.h>
#include "StateMachine.h"
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"
#include "ServoAccelerationController.h"
#include "CylinderControl.h"
#include <FastAccelStepper.h>

//* ************************************************************************
//* ************************ STORE STATE VARIABLES ***********************
//* ************************************************************************
static bool storeStateInitialized = false;
static bool storeComplete = false;
static ServoAccelerationController* storeServoController = NULL;
static CylinderControl* storeCylinder = NULL;
static FastAccelStepper* storeZMotor = NULL;
static int currentStep = 0;
static unsigned long stepStartTime = 0;
static int targetHeightSteps = 0;

//* ************************************************************************
//* ************************ STORE STATE FUNCTIONS ***********************
//* ************************************************************************

void executeStoreState() {
    //! ************************************************************************
    //! EXECUTE STORE STATE - STORAGE OPERATION WITH CYLINDER CONTROL
    //! ************************************************************************
    
    // Initialize store state on first entry
    if (!storeStateInitialized) {
        Serial.println("=== ENTERING STORE STATE ===");
        Serial.println("Starting storage operation...");
        storeStateInitialized = true;
        storeComplete = false;
        currentStep = 0;
        stepStartTime = millis();
        
        //! ************************************************************************
        //! STEP 0: SETUP INITIAL PARAMETERS
        //! ************************************************************************
        targetHeightSteps = STORE_HEIGHT_STEPS;
        
        // Set motor speed and acceleration
        if (storeZMotor) {
            storeZMotor->setSpeedInHz(Z_MAX_SPEED);
            storeZMotor->setAcceleration(Z_ACCELERATION);
        }
        
        // Set servo acceleration profile
        if (storeServoController) {
            storeServoController->setAccelerationProfile(300, 2000);
        }
    }
    
    //! ************************************************************************
    //! STEP 1: CHECK IF STORING IS COMPLETE
    //! ************************************************************************
    if (storeComplete) {
        Serial.println("Storage operation complete - transitioning to RETRIEVE 2 state");
        setState(RETRIEVE_2_STATE);
        return;
    }
    
    //! ************************************************************************
    //! STEP 2: PERFORM STORAGE SEQUENCE
    //! ************************************************************************
    unsigned long currentTime = millis();
    
    switch (currentStep) {
        case 0: {
            //! ************************************************************************
            //! STEP 0: MOVE TO SPECIFIC HEIGHT AND ANGLE
            //! ************************************************************************
            if (currentTime - stepStartTime >= 100) { // Small delay to ensure initialization
                Serial.println("Step 0: Moving to store position - Height: " + String(STORE_HEIGHT_INCHES) + " inches, Angle: " + String(STORE_ANGLE_DEGREES) + " degrees");
                
                // Move Z motor to store height
                if (storeZMotor) {
                    storeZMotor->moveTo(targetHeightSteps);
                }
                
                // Move servo to store angle
                if (storeServoController) {
                    storeServoController->moveTo(STORE_ANGLE_DEGREES);
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
            bool zMotorReady = !storeZMotor || !storeZMotor->isRunning();
            bool servoReady = !storeServoController || storeServoController->hasReachedTarget();
            
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
            if (storeCylinder) {
                storeCylinder->extend();
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
            if (storeZMotor) {
                int newHeightSteps = targetHeightSteps - HEIGHT_ADJUSTMENT_STEPS;
                storeZMotor->moveTo(newHeightSteps);
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
            bool heightAdjustmentComplete = !storeZMotor || !storeZMotor->isRunning();
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
            //! STEP 7: RETRACT THE CYLINDER
            //! ************************************************************************
            if (storeCylinder) {
                storeCylinder->retract();
                Serial.println("Step 7: Cylinder retracted");
            }
            currentStep = 8;
            stepStartTime = currentTime;
            break;
        }
            
        case 8: {
            //! ************************************************************************
            //! STEP 8: WAIT 750MS AFTER RETRACTING CYLINDER
            //! ************************************************************************
            if (currentTime - stepStartTime >= CYLINDER_RETRACT_WAIT) {
                Serial.println("Step 8: Cylinder retract wait complete, storage sequence finished");
                storeComplete = true;
            }
            break;
        }
            
        default: {
            Serial.println("ERROR: Invalid step in store sequence");
            storeComplete = true;
            break;
        }
    }
}

void resetStoreState() {
    //! ************************************************************************
    //! RESET STORE STATE FLAGS
    //! ************************************************************************
    storeStateInitialized = false;
    storeComplete = false;
    currentStep = 0;
    stepStartTime = 0;
    targetHeightSteps = 0;
}

void setStoreReferences(ServoAccelerationController* servoController, CylinderControl* cylinder, FastAccelStepper* zMotor) {
    //! ************************************************************************
    //! SET REFERENCES TO SERVO CONTROLLER, CYLINDER, AND Z MOTOR OBJECTS
    //! ************************************************************************
    storeServoController = servoController;
    storeCylinder = cylinder;
    storeZMotor = zMotor;
} 