//* ************************************************************************
//* ************************ CELL TEST STATE ****************************
//* ************************************************************************
//! CELL TEST state - Performs pick and place test for a specific cell
//! This state handles moving to a cell position and performing a complete pick/place cycle

#include <Arduino.h>
#include "StateMachine.h"
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"
#include "Config/CellConfig.h"
#include "ServoControl.h"
#include "LoaderForkStepper.h"
#include <FastAccelStepper.h>

//* ************************************************************************
//* ************************ CELL TEST STATE VARIABLES ******************
//* ************************************************************************
static bool cellTestStateInitialized = false;
static bool cellTestComplete = false;
static ServoControl* cellTestServoController = NULL;
static LoaderForkStepper* cellTestLoaderFork = NULL;
static FastAccelStepper* cellTestZMotor = NULL;
static int currentStep = 0;
static unsigned long stepStartTime = 0;
static char targetColumn = 'A';
static int targetRow = 1;
static CellPosition targetCellPosition;
static int targetHeightSteps = 0;
static bool pickPhaseComplete = false;
static bool placePhaseComplete = false;

//* ************************************************************************
//* ************************ CELL TEST STATE FUNCTIONS ******************
//* ************************************************************************

void executeCellTestState() {
    //! ************************************************************************
    //! EXECUTE CELL TEST STATE - CELL TESTING OPERATION
    //! ************************************************************************
    
    // Initialize cell test state on first entry
    if (!cellTestStateInitialized) {
        Serial.println("=== ENTERING CELL TEST STATE ===");
        Serial.println("Starting cell test for cell " + String(targetColumn) + String(targetRow));
        cellTestStateInitialized = true;
        cellTestComplete = false;
        currentStep = 0;
        stepStartTime = millis();
        pickPhaseComplete = false;
        placePhaseComplete = false;
        
        //! ************************************************************************
        //! STEP 0: GET CELL POSITION AND SETUP PARAMETERS
        //! ************************************************************************
        targetCellPosition = getCellPosition(targetColumn, targetRow);
        targetHeightSteps = getCellHeightSteps(targetColumn, targetRow);
        
        Serial.println("Cell " + String(targetColumn) + String(targetRow) + " configuration:");
        Serial.println("  Height: " + String(targetCellPosition.height_inches) + " inches");
        Serial.println("  Servo angle: " + String(targetCellPosition.servo_angle) + " degrees");
        Serial.println("  Height steps: " + String(targetHeightSteps));
        
        // Set motor speed and acceleration
        if (cellTestZMotor) {
            cellTestZMotor->setSpeedInHz(Z_MAX_SPEED);
            cellTestZMotor->setAcceleration(Z_ACCELERATION);
        }
    }
    
    //! ************************************************************************
    //! STEP 1: CHECK IF CELL TEST IS COMPLETE
    //! ************************************************************************
    if (cellTestComplete) {
        Serial.println("Cell test complete - transitioning to IDLE state");
        setState(IDLE_STATE);
        return;
    }
    
    //! ************************************************************************
    //! STEP 2: PERFORM CELL TEST SEQUENCE
    //! ************************************************************************
    unsigned long currentTime = millis();
    
    switch (currentStep) {
        case 0: {
            //! ************************************************************************
            //! STEP 0: MOVE TO CELL POSITION
            //! ************************************************************************
            if (currentTime - stepStartTime >= 100) { // Small delay to ensure initialization
                Serial.println("Step 0: Moving to cell " + String(targetColumn) + String(targetRow) + " position");
                
                // Move Z motor to cell height
                if (cellTestZMotor) {
                    cellTestZMotor->moveTo(targetHeightSteps);
                }
                
                // Move servo to cell angle
                if (cellTestServoController) {
                    cellTestServoController->write(targetCellPosition.servo_angle);
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
            bool zMotorReady = !cellTestZMotor || !cellTestZMotor->isRunning();
            bool servoReady = !cellTestServoController || cellTestServoController->isMoveComplete();
            
            if (zMotorReady && servoReady) {
                Serial.println("Step 1: Position reached, starting PICK phase");
                currentStep = 2;
                stepStartTime = currentTime;
            }
            break;
        }
            
        case 2: {
            //! ************************************************************************
            //! STEP 2: PERFORM PICK OPERATION
            //! ************************************************************************
            if (!pickPhaseComplete) {
                // Extend the loader fork to cell pick position
                if (cellTestLoaderFork) {
                    cellTestLoaderFork->extendToCellPickPosition();
                    Serial.println("Step 2: Loader fork extending to cell pick position");
                }
                currentStep = 3;
                stepStartTime = currentTime;
            } else {
                // Pick phase complete, move to place phase
                currentStep = 6;
                stepStartTime = currentTime;
            }
            break;
        }
            
        case 3: {
            //! ************************************************************************
            //! STEP 3: WAIT FOR FORK EXTENSION TO COMPLETE
            //! ************************************************************************
            bool forkExtensionComplete = !cellTestLoaderFork || !cellTestLoaderFork->isMoving();
            if (forkExtensionComplete) {
                Serial.println("Step 3: Fork extension complete, raising height by " + String(HEIGHT_ADJUSTMENT_INCHES) + " inches");
                currentStep = 4;
                stepStartTime = currentTime;
            }
            break;
        }
            
        case 4: {
            //! ************************************************************************
            //! STEP 4: RAISE HEIGHT BY ADJUSTMENT AMOUNT
            //! ************************************************************************
            if (cellTestZMotor) {
                int newHeightSteps = targetHeightSteps + HEIGHT_ADJUSTMENT_STEPS;
                cellTestZMotor->moveTo(newHeightSteps);
                Serial.println("Step 4: Raising height to " + String((float)newHeightSteps / STEPS_PER_INCH) + " inches");
            }
            currentStep = 5;
            stepStartTime = currentTime;
            break;
        }
            
        case 5: {
            //! ************************************************************************
            //! STEP 5: WAIT FOR HEIGHT ADJUSTMENT AND RETRACT FORK
            //! ************************************************************************
            bool heightAdjustmentComplete = !cellTestZMotor || !cellTestZMotor->isRunning();
            if (heightAdjustmentComplete) {
                // Retract the loader fork
                if (cellTestLoaderFork) {
                    cellTestLoaderFork->retract();
                    Serial.println("Step 5: Height adjustment complete, retracting fork");
                }
                currentStep = 6;
                stepStartTime = currentTime;
            }
            break;
        }
            
        case 6: {
            //! ************************************************************************
            //! STEP 6: WAIT FOR FORK RETRACTION AND COMPLETE PICK PHASE
            //! ************************************************************************
            bool forkRetractionComplete = !cellTestLoaderFork || !cellTestLoaderFork->isMoving();
            if (forkRetractionComplete) {
                Serial.println("Step 6: Fork retraction complete, PICK phase finished");
                pickPhaseComplete = true;
                currentStep = 7;
                stepStartTime = currentTime;
            }
            break;
        }
            
        case 7: {
            //! ************************************************************************
            //! STEP 7: PERFORM PLACE OPERATION
            //! ************************************************************************
            if (!placePhaseComplete) {
                // Extend the loader fork to cell place position
                if (cellTestLoaderFork) {
                    cellTestLoaderFork->extendToCellPlacePosition();
                    Serial.println("Step 7: Loader fork extending to cell place position");
                }
                currentStep = 8;
                stepStartTime = currentTime;
            } else {
                // Place phase complete, finish test
                currentStep = 11;
                stepStartTime = currentTime;
            }
            break;
        }
            
        case 8: {
            //! ************************************************************************
            //! STEP 8: WAIT FOR FORK EXTENSION TO COMPLETE
            //! ************************************************************************
            bool forkExtensionComplete = !cellTestLoaderFork || !cellTestLoaderFork->isMoving();
            if (forkExtensionComplete) {
                Serial.println("Step 8: Fork extension complete, lowering height by " + String(HEIGHT_ADJUSTMENT_INCHES) + " inches");
                currentStep = 9;
                stepStartTime = currentTime;
            }
            break;
        }
            
        case 9: {
            //! ************************************************************************
            //! STEP 9: LOWER HEIGHT BY ADJUSTMENT AMOUNT
            //! ************************************************************************
            if (cellTestZMotor) {
                int newHeightSteps = targetHeightSteps - HEIGHT_ADJUSTMENT_STEPS;
                cellTestZMotor->moveTo(newHeightSteps);
                Serial.println("Step 9: Lowering height to " + String((float)newHeightSteps / STEPS_PER_INCH) + " inches");
            }
            currentStep = 10;
            stepStartTime = currentTime;
            break;
        }
            
        case 10: {
            //! ************************************************************************
            //! STEP 10: WAIT FOR HEIGHT ADJUSTMENT AND RETRACT FORK
            //! ************************************************************************
            bool heightAdjustmentComplete = !cellTestZMotor || !cellTestZMotor->isRunning();
            if (heightAdjustmentComplete) {
                // Retract the loader fork
                if (cellTestLoaderFork) {
                    cellTestLoaderFork->retract();
                    Serial.println("Step 10: Height adjustment complete, retracting fork");
                }
                currentStep = 11;
                stepStartTime = currentTime;
            }
            break;
        }
            
        case 11: {
            //! ************************************************************************
            //! STEP 11: WAIT FOR FORK RETRACTION AND COMPLETE TEST
            //! ************************************************************************
            bool forkRetractionComplete = !cellTestLoaderFork || !cellTestLoaderFork->isMoving();
            if (forkRetractionComplete) {
                Serial.println("Step 11: Fork retraction complete, PLACE phase finished");
                placePhaseComplete = true;
                cellTestComplete = true;
            }
            break;
        }
            
        default: {
            Serial.println("ERROR: Invalid step in cell test sequence");
            cellTestComplete = true;
            break;
        }
    }
}

void resetCellTestState() {
    //! ************************************************************************
    //! RESET CELL TEST STATE FLAGS
    //! ************************************************************************
    cellTestStateInitialized = false;
    cellTestComplete = false;
    currentStep = 0;
    stepStartTime = 0;
    targetColumn = 'A';
    targetRow = 1;
    targetHeightSteps = 0;
    pickPhaseComplete = false;
    placePhaseComplete = false;
}

void setCellTestReferences(ServoControl* servoController, LoaderForkStepper* loaderFork, FastAccelStepper* zMotor) {
    //! ************************************************************************
    //! SET REFERENCES TO SERVO CONTROLLER, LOADER FORK, AND Z MOTOR OBJECTS
    //! ************************************************************************
    cellTestServoController = servoController;
    cellTestLoaderFork = loaderFork;
    cellTestZMotor = zMotor;
}

void setCellTestTarget(char column, int row) {
    //! ************************************************************************
    //! SET THE TARGET CELL FOR TESTING
    //! ************************************************************************
    if (isValidCell(column, row)) {
        targetColumn = column;
        targetRow = row;
        Serial.println("Cell test target set to: " + String(targetColumn) + String(targetRow));
    } else {
        Serial.println("ERROR: Invalid cell reference for testing: " + String(column) + String(row));
    }
} 