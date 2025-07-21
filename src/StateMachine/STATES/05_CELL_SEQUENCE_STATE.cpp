#include <Arduino.h>
#include "StateMachine.h"
#include "Config/CellConfig.h"
#include "Config/Config.h"
#include "ServoAccelerationController.h"
#include "LoaderForkStepper.h"

//* ************************************************************************
//* ************************ CELL SEQUENCE STATE **************************
//* ************************************************************************
// Automated sequence: Pick from loading tray, then place in each cell (A1-D5)
// Pick: Move to 0.3" below cell height, extend fork 4.1", retract fork
// Place: Move to 0.3" above cell height, lower down, retract fork
// Loading tray: Skip 0.3" adjustment (already positioned correctly)

//* ************************************************************************
//* ************************ GLOBAL VARIABLES *****************************
//* ************************************************************************
static CellSequenceData sequenceData;
static ServoAccelerationController* cellSequenceServoController = nullptr;
static LoaderForkStepper* cellSequenceLoaderFork = nullptr;
static FastAccelStepper* cellSequenceZMotor = nullptr;

//* ************************************************************************
//* ************************ CONSTANTS ***********************************
//* ************************************************************************
#define HEIGHT_OFFSET_INCHES 0.3
#define LOADING_TRAY_HEIGHT_INCHES 0.3  // Adjust this to match your loading tray height
#define LOADING_TRAY_ANGLE_DEGREES 48.0   // Adjust this to match your loading tray angle

//* ************************************************************************
//* ************************ STATE EXECUTION *****************************
//* ************************************************************************
void executeCellSequenceState() {
    //! ************************************************************************
    //! STEP 1: CHECK IF SEQUENCE IS COMPLETE
    //! ************************************************************************
    if (sequenceData.sequenceComplete) {
        Serial.println("Cell sequence complete - returning to IDLE state");
        setState(IDLE_STATE);
        return;
    }

    //! ************************************************************************
    //! STEP 2: EXECUTE CURRENT OPERATION
    //! ************************************************************************
    if (sequenceData.isPicking) {
        performPickOperation();
    } else {
        performPlaceOperation();
    }
}

//* ************************************************************************
//* ************************ SEQUENCE CONTROL ****************************
//* ************************************************************************
void startCellSequence() {
    //! ************************************************************************
    //! INITIALIZE CELL SEQUENCE
    //! ************************************************************************
    sequenceData.currentColumn = 'A';
    sequenceData.currentRow = 1;
    sequenceData.isPicking = true;  // Start by picking from loading tray
    sequenceData.sequenceComplete = false;
    sequenceData.currentStep = 0;
    sequenceData.stepStartTime = millis();
    
    Serial.println("=== STARTING CELL SEQUENCE ===");
    Serial.println("Sequence: Pick from loading tray, then place in cells A1-D5");
}

void nextCell() {
    //! ************************************************************************
    //! ADVANCE TO NEXT CELL
    //! ************************************************************************
    sequenceData.currentRow++;
    if (sequenceData.currentRow > TOTAL_ROWS) {
        sequenceData.currentRow = 1;
        sequenceData.currentColumn++;
        if (sequenceData.currentColumn > 'D') {
            // All cells processed
            sequenceData.sequenceComplete = true;
            Serial.println("All cells processed - sequence complete");
            return;
        }
    }
    
    // Reset for next cell
    sequenceData.isPicking = true;
    sequenceData.currentStep = 0;
    sequenceData.stepStartTime = millis();
    
    Serial.println("Moving to cell " + String(sequenceData.currentColumn) + String(sequenceData.currentRow));
}

void continueCellSequence() {
    //! ************************************************************************
    //! CONTINUE SEQUENCE AFTER START BUTTON PRESS
    //! ************************************************************************
    if (sequenceData.currentStep == 4) {
        // We were waiting at loading tray position, now lift fork to catch square
        sequenceData.currentStep = 5;
        sequenceData.stepStartTime = millis();
    } else if (sequenceData.currentStep == 8) {
        // We were waiting for start button press, now continue to next cell
        nextCell();
    }
}

//* ************************************************************************
//* ************************ PICK OPERATION ******************************
//* ************************************************************************
void performPickOperation() {
    unsigned long currentTime = millis();
    
    switch (sequenceData.currentStep) {
        case 0: {
            //! ************************************************************************
            //! STEP 0: MOVE TO LOADING TRAY POSITION
            //! ************************************************************************
            if (currentTime - sequenceData.stepStartTime >= 100) {
                Serial.println("Step 0: Moving to loading tray position");
                
                // Move Z motor to loading tray height (no offset needed)
                if (cellSequenceZMotor) {
                    int targetSteps = (int)(LOADING_TRAY_HEIGHT_INCHES * STEPS_PER_INCH);
                    cellSequenceZMotor->setSpeedInHz(Z_MAX_SPEED);
                    cellSequenceZMotor->setAcceleration(Z_ACCELERATION);
                    cellSequenceZMotor->moveTo(targetSteps);
                }
                
                // Move servo to loading tray angle
                if (cellSequenceServoController) {
                    cellSequenceServoController->moveTo(LOADING_TRAY_ANGLE_DEGREES);
                }
                
                sequenceData.currentStep = 1;
                sequenceData.stepStartTime = currentTime;
            }
            break;
        }
        
        case 1: {
            //! ************************************************************************
            //! STEP 1: WAIT FOR MOTOR AND SERVO TO REACH POSITION
            //! ************************************************************************
            bool zMotorReady = !cellSequenceZMotor || !cellSequenceZMotor->isRunning();
            bool servoReady = !cellSequenceServoController || cellSequenceServoController->isMoveComplete();
            
            if (zMotorReady && servoReady) {
                Serial.println("Step 1: Loading tray position reached, extending fork");
                sequenceData.currentStep = 2;
                sequenceData.stepStartTime = currentTime;
            }
            break;
        }
        
        case 2: {
            //! ************************************************************************
            //! STEP 2: EXTEND FORK TO PICK POSITION
            //! ************************************************************************
            if (cellSequenceLoaderFork) {
                cellSequenceLoaderFork->extendToPickPosition();
                Serial.println("Step 2: Fork extending to pick position");
            }
            sequenceData.currentStep = 3;
            sequenceData.stepStartTime = currentTime;
            break;
        }
        
        case 3: {
            //! ************************************************************************
            //! STEP 3: WAIT FOR FORK EXTENSION TO COMPLETE
            //! ************************************************************************
            bool forkExtensionComplete = !cellSequenceLoaderFork || !cellSequenceLoaderFork->isMoving();
            if (forkExtensionComplete) {
                Serial.println("Step 3: Fork extension complete, waiting at loading tray position");
                Serial.println("Press START button to lift fork and catch square...");
                sequenceData.currentStep = 4; // Wait for start button press
                sequenceData.stepStartTime = currentTime;
            }
            break;
        }
        
        case 4: {
            //! ************************************************************************
            //! STEP 4: WAIT FOR START BUTTON PRESS TO LIFT FORK
            //! ************************************************************************
            // This step waits indefinitely until the start button is pressed again
            // The start button handling is done in main.cpp
            break;
        }
        
        case 5: {
            //! ************************************************************************
            //! STEP 5: LIFT FORK 0.5 INCHES TO CATCH THE WOOD SQUARE
            //! ************************************************************************
            if (cellSequenceZMotor) {
                int currentSteps = cellSequenceZMotor->getCurrentPosition();
                int liftSteps = (int)(0.5 * STEPS_PER_INCH);
                int targetSteps = currentSteps + liftSteps;
                
                cellSequenceZMotor->setSpeedInHz(Z_MAX_SPEED);
                cellSequenceZMotor->setAcceleration(Z_ACCELERATION);
                cellSequenceZMotor->moveTo(targetSteps);
                Serial.println("Step 5: Lifting fork 0.5 inches to catch square");
            }
            sequenceData.currentStep = 6;
            sequenceData.stepStartTime = currentTime;
            break;
        }
        
        case 6: {
            //! ************************************************************************
            //! STEP 6: WAIT FOR LIFTING TO COMPLETE
            //! ************************************************************************
            bool liftingComplete = !cellSequenceZMotor || !cellSequenceZMotor->isRunning();
            if (liftingComplete) {
                Serial.println("Step 6: Lifting complete, retracting fork");
                sequenceData.currentStep = 7;
                sequenceData.stepStartTime = currentTime;
            }
            break;
        }
        
        case 7: {
            //! ************************************************************************
            //! STEP 7: RETRACT FORK
            //! ************************************************************************
            if (cellSequenceLoaderFork) {
                cellSequenceLoaderFork->retract();
                Serial.println("Step 7: Fork retracting");
            }
            sequenceData.currentStep = 8;
            sequenceData.stepStartTime = currentTime;
            break;
        }
        
        case 8: {
            //! ************************************************************************
            //! STEP 8: WAIT FOR FORK RETRACTION TO COMPLETE
            //! ************************************************************************
            bool forkRetractionComplete = !cellSequenceLoaderFork || !cellSequenceLoaderFork->isMoving();
            if (forkRetractionComplete) {
                Serial.println("Step 8: Pick operation complete, switching to place mode");
                sequenceData.isPicking = false;
                sequenceData.currentStep = 0;
                sequenceData.stepStartTime = currentTime;
            }
            break;
        }
    }
}

//* ************************************************************************
//* ************************ PLACE OPERATION *****************************
//* ************************************************************************
void performPlaceOperation() {
    unsigned long currentTime = millis();
    
    switch (sequenceData.currentStep) {
        case 0: {
            //! ************************************************************************
            //! STEP 0: MOVE TO CELL POSITION (0.3" ABOVE)
            //! ************************************************************************
            if (currentTime - sequenceData.stepStartTime >= 100) {
                CellPosition cellPos = getCellPosition(sequenceData.currentColumn, sequenceData.currentRow);
                float targetHeight = cellPos.height_inches + HEIGHT_OFFSET_INCHES;
                
                Serial.println("Step 0: Moving to cell " + String(sequenceData.currentColumn) + String(sequenceData.currentRow) + 
                              " position (0.3\" above)");
                
                // Move Z motor to cell height + offset
                if (cellSequenceZMotor) {
                    int targetSteps = (int)(targetHeight * STEPS_PER_INCH);
                    cellSequenceZMotor->setSpeedInHz(Z_MAX_SPEED);
                    cellSequenceZMotor->setAcceleration(Z_ACCELERATION);
                    cellSequenceZMotor->moveTo(targetSteps);
                }
                
                // Move servo to cell angle
                if (cellSequenceServoController) {
                    cellSequenceServoController->moveTo(cellPos.servo_angle);
                }
                
                sequenceData.currentStep = 1;
                sequenceData.stepStartTime = currentTime;
            }
            break;
        }
        
        case 1: {
            //! ************************************************************************
            //! STEP 1: WAIT FOR MOTOR AND SERVO TO REACH POSITION
            //! ************************************************************************
            bool zMotorReady = !cellSequenceZMotor || !cellSequenceZMotor->isRunning();
            bool servoReady = !cellSequenceServoController || cellSequenceServoController->isMoveComplete();
            
            if (zMotorReady && servoReady) {
                Serial.println("Step 1: Cell position reached, extending fork");
                sequenceData.currentStep = 2;
                sequenceData.stepStartTime = currentTime;
            }
            break;
        }
        
        case 2: {
            //! ************************************************************************
            //! STEP 2: EXTEND FORK TO PLACE POSITION
            //! ************************************************************************
            if (cellSequenceLoaderFork) {
                cellSequenceLoaderFork->extendToPlacePosition();
                Serial.println("Step 2: Fork extending to place position");
            }
            sequenceData.currentStep = 3;
            sequenceData.stepStartTime = currentTime;
            break;
        }
        
        case 3: {
            //! ************************************************************************
            //! STEP 3: WAIT FOR FORK EXTENSION TO COMPLETE
            //! ************************************************************************
            bool forkExtensionComplete = !cellSequenceLoaderFork || !cellSequenceLoaderFork->isMoving();
            if (forkExtensionComplete) {
                Serial.println("Step 3: Fork extension complete, lowering to cell height");
                sequenceData.currentStep = 4;
                sequenceData.stepStartTime = currentTime;
            }
            break;
        }
        
        case 4: {
            //! ************************************************************************
            //! STEP 4: LOWER TO CELL HEIGHT
            //! ************************************************************************
            CellPosition cellPos = getCellPosition(sequenceData.currentColumn, sequenceData.currentRow);
            
            if (cellSequenceZMotor) {
                int targetSteps = (int)(cellPos.height_inches * STEPS_PER_INCH);
                cellSequenceZMotor->setSpeedInHz(Z_MAX_SPEED);
                cellSequenceZMotor->setAcceleration(Z_ACCELERATION);
                cellSequenceZMotor->moveTo(targetSteps);
                Serial.println("Step 4: Lowering to cell height: " + String(cellPos.height_inches) + " inches");
            }
            sequenceData.currentStep = 5;
            sequenceData.stepStartTime = currentTime;
            break;
        }
        
        case 5: {
            //! ************************************************************************
            //! STEP 5: WAIT FOR LOWERING TO COMPLETE
            //! ************************************************************************
            bool loweringComplete = !cellSequenceZMotor || !cellSequenceZMotor->isRunning();
            if (loweringComplete) {
                Serial.println("Step 5: Lowering complete, retracting fork");
                sequenceData.currentStep = 6;
                sequenceData.stepStartTime = currentTime;
            }
            break;
        }
        
        case 6: {
            //! ************************************************************************
            //! STEP 6: RETRACT FORK
            //! ************************************************************************
            if (cellSequenceLoaderFork) {
                cellSequenceLoaderFork->retract();
                Serial.println("Step 6: Fork retracting");
            }
            sequenceData.currentStep = 7;
            sequenceData.stepStartTime = currentTime;
            break;
        }
        
        case 7: {
            //! ************************************************************************
            //! STEP 7: WAIT FOR FORK RETRACTION TO COMPLETE
            //! ************************************************************************
            bool forkRetractionComplete = !cellSequenceLoaderFork || !cellSequenceLoaderFork->isMoving();
            if (forkRetractionComplete) {
                Serial.println("Step 7: Place operation complete for cell " + String(sequenceData.currentColumn) + String(sequenceData.currentRow));
                Serial.println("Press START button to continue to next cell...");
                sequenceData.currentStep = 8; // Wait for start button press
                sequenceData.stepStartTime = currentTime;
            }
            break;
        }
        
        case 8: {
            //! ************************************************************************
            //! STEP 8: WAIT FOR START BUTTON PRESS TO CONTINUE
            //! ************************************************************************
            // This step waits indefinitely until the start button is pressed again
            // The start button handling is done in main.cpp
            break;
        }
    }
}

//* ************************************************************************
//* ************************ STATE MANAGEMENT ****************************
//* ************************************************************************
void resetCellSequenceState() {
    //! ************************************************************************
    //! RESET CELL SEQUENCE STATE
    //! ************************************************************************
    sequenceData.sequenceComplete = false;
    sequenceData.currentStep = 0;
    sequenceData.stepStartTime = 0;
}

void setCellSequenceReferences(ServoAccelerationController* servoController, LoaderForkStepper* loaderFork, FastAccelStepper* zMotor) {
    //! ************************************************************************
    //! SET REFERENCES FOR CELL SEQUENCE STATE
    //! ************************************************************************
    cellSequenceServoController = servoController;
    cellSequenceLoaderFork = loaderFork;
    cellSequenceZMotor = zMotor;
} 