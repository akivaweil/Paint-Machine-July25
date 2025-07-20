//* ************************************************************************
//* ************************ STORE 2 STATE ********************************
//* ************************************************************************
//! STORE 2 state - Performs second storing operations with cylinder control
//! This state handles the second storage sequence: move to position, extend cylinder, adjust height, retract

#include <Arduino.h>
#include "StateMachine.h"
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"
#include "ServoAccelerationController.h"
#include "CylinderControl.h"
#include <FastAccelStepper.h>

//* ************************************************************************
//* ************************ STORE 2 STATE VARIABLES **********************
//* ************************************************************************
static bool store2StateInitialized = false;
static bool store2Complete = false;
static ServoAccelerationController* store2ServoController = NULL;
static CylinderControl* store2Cylinder = NULL;
static FastAccelStepper* store2ZMotor = NULL;
static int currentStep = 0;
static unsigned long stepStartTime = 0;
static int targetHeightSteps = 0;

//* ************************************************************************
//* ************************ STORE 2 STATE FUNCTIONS **********************
//* ************************************************************************

void executeStore2State() {
    //! ************************************************************************
    //! EXECUTE STORE 2 STATE - SECOND STORAGE OPERATION WITH CYLINDER CONTROL
    //! ************************************************************************
    
    // Initialize store 2 state on first entry
    if (!store2StateInitialized) {
        Serial.println("=== ENTERING STORE 2 STATE ===");
        Serial.println("Starting second storage operation...");
        store2StateInitialized = true;
        store2Complete = false;
        currentStep = 0;
        stepStartTime = millis();
        
        //! ************************************************************************
        //! STEP 0: SETUP INITIAL PARAMETERS
        //! ************************************************************************
        targetHeightSteps = STORE_2_HEIGHT_STEPS;
        
        // Set motor speed and acceleration
        if (store2ZMotor) {
            store2ZMotor->setSpeedInHz(Z_MAX_SPEED);
            store2ZMotor->setAcceleration(Z_ACCELERATION);
        }
        
        // Set servo acceleration profile
        if (store2ServoController) {
            store2ServoController->setAccelerationProfile(300, 2000);
        }
    }
    
    //! ************************************************************************
    //! STEP 1: CHECK IF STORING IS COMPLETE
    //! ************************************************************************
    if (store2Complete) {
        Serial.println("Second storage operation complete - transitioning to IDLE state");
        setState(IDLE_STATE);
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
                Serial.println("Step 0: Moving to second store position - Height: " + String(STORE_2_HEIGHT_INCHES) + " inches, Angle: " + String(STORE_2_ANGLE_DEGREES) + " degrees");
                
                // Move Z motor to store 2 height
                if (store2ZMotor) {
                    store2ZMotor->moveTo(targetHeightSteps);
                }
                
                // Move servo to store 2 angle
                if (store2ServoController) {
                    store2ServoController->moveTo(STORE_2_ANGLE_DEGREES);
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
            bool zMotorReady = !store2ZMotor || !store2ZMotor->isRunning();
            bool servoReady = !store2ServoController || store2ServoController->hasReachedTarget();
            
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
            if (store2Cylinder) {
                store2Cylinder->extend();
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
            if (store2ZMotor) {
                int newHeightSteps = targetHeightSteps - HEIGHT_ADJUSTMENT_STEPS;
                store2ZMotor->moveTo(newHeightSteps);
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
            bool heightAdjustmentComplete = !store2ZMotor || !store2ZMotor->isRunning();
            if (heightAdjustmentComplete) {
                Serial.println("Step 5: Height adjustment complete, retracting cylinder");
                currentStep = 6;
                stepStartTime = currentTime;
            }
            break;
        }
            
        case 6: {
            //! ************************************************************************
            //! STEP 6: RETRACT THE CYLINDER
            //! ************************************************************************
            if (store2Cylinder) {
                store2Cylinder->retract();
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
                Serial.println("Step 7: Cylinder retract wait complete, second storage sequence finished");
                store2Complete = true;
            }
            break;
        }
            
        default: {
            Serial.println("ERROR: Invalid step in store 2 sequence");
            store2Complete = true;
            break;
        }
    }
}

void resetStore2State() {
    //! ************************************************************************
    //! RESET STORE 2 STATE FLAGS
    //! ************************************************************************
    store2StateInitialized = false;
    store2Complete = false;
    currentStep = 0;
    stepStartTime = 0;
    targetHeightSteps = 0;
}

void setStore2References(ServoAccelerationController* servoController, CylinderControl* cylinder, FastAccelStepper* zMotor) {
    //! ************************************************************************
    //! SET REFERENCES TO SERVO CONTROLLER, CYLINDER, AND Z MOTOR OBJECTS
    //! ************************************************************************
    store2ServoController = servoController;
    store2Cylinder = cylinder;
    store2ZMotor = zMotor;
} 