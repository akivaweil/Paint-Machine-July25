//* ************************************************************************
//* ************************ RETRIEVE STATE ******************************
//* ************************************************************************
//! RETRIEVE state - Performs retrieving operations using servo movements
//! This state handles the retrieval sequence for the paint machine using servo control

#include <Arduino.h>
#include "StateMachine.h"
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"
#include "ServoControl.h"

//* ************************************************************************
//* ************************ RETRIEVE STATE VARIABLES ********************
//* ************************************************************************
static bool retrieveStateInitialized = false;
static bool retrieveComplete = false;
static ServoControl* retrieveServo = NULL;
static int currentStep = 0;
static unsigned long stepStartTime = 0;
static const unsigned long STEP_DELAY = 1000; // 1 second delay between steps

//* ************************************************************************
//* ************************ RETRIEVE STATE FUNCTIONS ********************
//* ************************************************************************

void executeRetrieveState() {
    //! ************************************************************************
    //! EXECUTE RETRIEVE STATE - SERVO RETRIEVAL OPERATION
    //! ************************************************************************
    
    // Initialize retrieve state on first entry
    if (!retrieveStateInitialized) {
        Serial.println("=== ENTERING RETRIEVE STATE ===");
        Serial.println("Starting servo-based retrieval operation...");
        retrieveStateInitialized = true;
        retrieveComplete = false;
        currentStep = 0;
        stepStartTime = millis();
    }
    
    //! ************************************************************************
    //! STEP 1: CHECK IF RETRIEVING IS COMPLETE
    //! ************************************************************************
    if (retrieveComplete) {
        Serial.println("Retrieval operation complete - transitioning to IDLE");
        setState(IDLE_STATE);
        return;
    }
    
    //! ************************************************************************
    //! STEP 2: PERFORM SERVO RETRIEVING SEQUENCE
    //! ************************************************************************
    if (retrieveServo) {
        unsigned long currentTime = millis();
        
        //! ************************************************************************
        //! STEP 2A: EXECUTE STEP-BASED SEQUENCE
        //! ************************************************************************
        switch (currentStep) {
            case 0:
                //! ************************************************************************
                //! STEP 0: MOVE TO STARTING POSITION (90 degrees)
                //! ************************************************************************
                if (currentTime - stepStartTime >= STEP_DELAY) {
                    Serial.println("Step 0: Moving servo to starting position (90°)");
                    retrieveServo->write(90.0);
                    currentStep = 1;
                    stepStartTime = currentTime;
                }
                break;
                
            case 1:
                //! ************************************************************************
                //! STEP 1: MOVE TO RETRIEVAL POSITION (45 degrees)
                //! ************************************************************************
                if (currentTime - stepStartTime >= STEP_DELAY && retrieveServo->hasReachedTarget()) {
                    Serial.println("Step 1: Moving servo to retrieval position (45°)");
                    retrieveServo->write(45.0);
                    currentStep = 2;
                    stepStartTime = currentTime;
                }
                break;
                
            case 2:
                //! ************************************************************************
                //! STEP 2: PERFORM RETRIEVAL ACTION (30 degrees)
                //! ************************************************************************
                if (currentTime - stepStartTime >= STEP_DELAY && retrieveServo->hasReachedTarget()) {
                    Serial.println("Step 2: Performing retrieval action (30°)");
                    retrieveServo->write(30.0);
                    currentStep = 3;
                    stepStartTime = currentTime;
                }
                break;
                
            case 3:
                //! ************************************************************************
                //! STEP 3: RETURN TO INTERMEDIATE POSITION (60 degrees)
                //! ************************************************************************
                if (currentTime - stepStartTime >= STEP_DELAY && retrieveServo->hasReachedTarget()) {
                    Serial.println("Step 3: Returning to intermediate position (60°)");
                    retrieveServo->write(60.0);
                    currentStep = 4;
                    stepStartTime = currentTime;
                }
                break;
                
            case 4:
                //! ************************************************************************
                //! STEP 4: RETURN TO FINAL POSITION (90 degrees)
                //! ************************************************************************
                if (currentTime - stepStartTime >= STEP_DELAY && retrieveServo->hasReachedTarget()) {
                    Serial.println("Step 4: Returning to final position (90°)");
                    retrieveServo->write(90.0);
                    currentStep = 5;
                    stepStartTime = currentTime;
                }
                break;
                
            case 5:
                //! ************************************************************************
                //! STEP 5: COMPLETE RETRIEVAL SEQUENCE
                //! ************************************************************************
                if (currentTime - stepStartTime >= STEP_DELAY && retrieveServo->hasReachedTarget()) {
                    Serial.println("Step 5: Retrieval sequence completed");
                    retrieveComplete = true;
                }
                break;
                
            default:
                Serial.println("ERROR: Invalid step in retrieve sequence");
                retrieveComplete = true;
                break;
        }
    } else {
        Serial.println("ERROR: Servo not available for retrieve state");
        setState(IDLE_STATE);
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
}

void setRetrieveReferences(ServoControl* servo) {
    //! ************************************************************************
    //! SET REFERENCES TO SERVO OBJECTS
    //! ************************************************************************
    retrieveServo = servo;
} 