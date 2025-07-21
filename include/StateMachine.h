#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include <FastAccelStepper.h>
#include <Bounce2.h>

// Forward declarations
class ServoControl;
class ServoAccelerationController;
class LoaderForkStepper;

//* ************************************************************************
//* ************************ STATE DEFINITIONS ****************************
//* ************************************************************************
enum StateMachineState {
    IDLE_STATE = 0,        // Waiting for commands
    HOME_STATE = 1,        // Homing sequence
    PICK_STATE = 2,        // Picking operation
    PLACE_STATE = 3,       // Placing operation
    TEST_STATE = 4,        // Test state for manual testing
    CELL_SEQUENCE_STATE = 5 // Automated cell sequence (pick from loading tray, place in cells)
};

//* ************************************************************************
//* ************************ CELL SEQUENCE VARIABLES **********************
//* ************************************************************************
struct CellSequenceData {
    char currentColumn;    // Current column (A, B, C, D)
    int currentRow;        // Current row (1, 2, 3, 4, 5)
    bool isPicking;        // true = picking from loading tray, false = placing in cell
    bool sequenceComplete; // true when all cells are processed
    int currentStep;       // Current step in the sequence
    unsigned long stepStartTime; // Timing for steps
    bool forkAlreadyExtended; // true if fork is already extended when starting place operation
};

//* ************************************************************************
//* ************************ STATE MACHINE FUNCTIONS **********************
//* ************************************************************************

// Main state machine functions
void initStateMachine();
void updateStateMachine();
void setState(StateMachineState newState);
StateMachineState getCurrentState();
StateMachineState getPreviousState();
bool isStateChanged();
String getStateName(StateMachineState state);

// Individual state execution functions
void executeIdleState();
void executeHomeState();
void executePickState();
void executePlaceState();
void executeTestState();
void executeCellSequenceState();

// State reset functions
void resetIdleState();
void resetHomeState();
void resetPickState();
void resetPlaceState();
void resetTestState();
void resetCellSequenceState();

// Home state specific functions
void performTestMotionSequence();

// State reference setup functions
void setHomeReferences(FastAccelStepper* motor, Bounce2::Button* homeSwitch);
// Overload to set loader fork stepper for homing
void setHomeReferences(FastAccelStepper* motor, Bounce2::Button* homeSwitch, LoaderForkStepper* forkStepper);
void setPickReferences(ServoControl* servoController, LoaderForkStepper* loaderFork, FastAccelStepper* zMotor);
void setPlaceReferences(ServoControl* servoController, LoaderForkStepper* loaderFork, FastAccelStepper* zMotor);
void setTestReferences(FastAccelStepper* motor, ServoAccelerationController* servoController, LoaderForkStepper* loaderFork);
void setIdleReferences(ServoControl* servo, ServoAccelerationController* servoController, FastAccelStepper* zMotor);
void setCellSequenceReferences(ServoControl* servoController, LoaderForkStepper* loaderFork, FastAccelStepper* zMotor);

// Cell sequence functions
void startCellSequence();
void nextCell();
void continueCellSequence();
void performPickOperation();
void performPlaceOperation();

// Test state manual mode functions
void parseManualCommand(String command);
void moveToManualHeight(float heightInches);
void moveToManualAngle(int angleDegrees);
void moveToManualForkPosition(float positionInches);
void printManualModeHelp();
void printManualModeStatus();
void toggleManualMode();
void checkSerialCommands();

#endif // STATE_MACHINE_H 