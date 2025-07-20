//* ************************************************************************
//* ************************ RETRIEVE 2 STATE ****************************
//* ************************************************************************
//! RETRIEVE 2 state - Performs second retrieving operations with cylinder control
//! This state handles the second retrieval sequence: move to position, extend cylinder, adjust height, retract

#include <Arduino.h>
#include "StateMachine.h"
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"
#include "ServoAccelerationController.h"
#include "CylinderControl.h"
#include <FastAccelStepper.h>

//* ************************************************************************
//* ************************ RETRIEVE 2 STATE VARIABLES *******************
//* ************************************************************************
static bool retrieve2StateInitialized = false;
static bool retrieve2Complete = false;
static ServoAccelerationController* retrieve2ServoController = NULL;
static CylinderControl* retrieve2Cylinder = NULL;
static FastAccelStepper* retrieve2ZMotor = NULL;
static int currentStep = 0;
static unsigned long stepStartTime = 0;
static int targetHeightSteps = 0;

//* ************************************************************************
//* ************************ RETRIEVE 2 STATE FUNCTIONS *******************
//* ************************************************************************

void executeRetrieve2State() {
    //! ************************************************************************
    //! EXECUTE RETRIEVE 2 STATE - SECOND RETRIEVAL OPERATION WITH CYLINDER CONTROL
    //! ************************************************************************
    
    // Initialize retrieve 2 state on first entry
    if (!retrieve2StateInitialized) {
        Serial.println("=== ENTERING RETRIEVE 2 STATE ===");
        Serial.println("Starting second retrieval operation...");
        retrieve2StateInitialized = true;
        retrieve2Complete = false;
        currentStep = 0;
        stepStartTime = millis();
        
        //! ************************************************************************
        //! STEP 0: SETUP INITIAL PARAMETERS
        //! ************************************************************************
        targetHeightSteps = RETRIEVE_2_HEIGHT_STEPS;
        
        // Set motor speed and acceleration
        if (retrieve2ZMotor) {
            retrieve2ZMotor->setSpeedInHz(Z_MAX_SPEED);
            retrieve2ZMotor->setAcceleration(Z_ACCELERATION);
        }
        
        // Set servo acceleration profile
        if (retrieve2ServoController) {
            retrieve2ServoController->setAccelerationProfile(300, 2000);
        }
    }
    
    //! ************************************************************************
    //! STEP 1: CHECK IF RETRIEVING IS COMPLETE
    //! ************************************************************************
    if (retrieve2Complete) {
        Serial.println("Second retrieval operation complete - transitioning to STORE 2 state");
        setState(STORE_2_STATE);
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
                Serial.println("Step 0: Moving to second retrieve position - Height: " + String(RETRIEVE_2_HEIGHT_INCHES) + " inches, Angle: " + String(RETRIEVE_2_ANGLE_DEGREES) + " degrees");
                
                // Move Z motor to retrieve 2 height
                if (retrieve2ZMotor) {
                    retrieve2ZMotor->moveTo(targetHeightSteps);
                }
                
                // Move servo to retrieve 2 angle
                if (retrieve2ServoController) {
                    retrieve2ServoController->moveTo(RETRIEVE_2_ANGLE_DEGREES);
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
            bool zMotorReady = !retrieve2ZMotor || !retrieve2ZMotor->isRunning();
            bool servoReady = !retrieve2ServoController || retrieve2ServoController->hasReachedTarget();
            
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
            if (retrieve2Cylinder) {
                retrieve2Cylinder->extend();
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
            //! STEP 4: RAISE HEIGHT BY .7 INCHES
            //! ************************************************************************
            if (retrieve2ZMotor) {
                int newHeightSteps = targetHeightSteps + HEIGHT_ADJUSTMENT_STEPS;
                retrieve2ZMotor->moveTo(newHeightSteps);
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
            if (retrieve2Cylinder) {
                retrieve2Cylinder->retract();
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
                Serial.println("Step 7: Cylinder retract wait complete, second retrieval sequence finished");
                retrieve2Complete = true;
            }
            break;
        }
            
        default: {
            Serial.println("ERROR: Invalid step in retrieve 2 sequence");
            retrieve2Complete = true;
            break;
        }
    }
}

void resetRetrieve2State() {
    //! ************************************************************************
    //! RESET RETRIEVE 2 STATE FLAGS
    //! ************************************************************************
    retrieve2StateInitialized = false;
    retrieve2Complete = false;
    currentStep = 0;
    stepStartTime = 0;
    targetHeightSteps = 0;
}

void setRetrieve2References(ServoAccelerationController* servoController, CylinderControl* cylinder, FastAccelStepper* zMotor) {
    //! ************************************************************************
    //! SET REFERENCES TO SERVO CONTROLLER, CYLINDER, AND Z MOTOR OBJECTS
    //! ************************************************************************
    retrieve2ServoController = servoController;
    retrieve2Cylinder = cylinder;
    retrieve2ZMotor = zMotor;
} 