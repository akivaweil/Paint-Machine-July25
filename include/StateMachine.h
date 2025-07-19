#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include <FastAccelStepper.h>
#include <Bounce2.h>

// Forward declarations
class ServoControl;
class ServoAccelerationController;
class CylinderControl;

//* ************************************************************************
//* ************************ STATE DEFINITIONS ****************************
//* ************************************************************************
enum StateMachineState {
    IDLE_STATE = 0,        // Waiting for commands
    HOME_STATE = 1,        // Homing sequence
    RETRIEVE_STATE = 2,    // Retrieving operation
    STORE_STATE = 3,       // Storing operation
    TEST_STATE = 4         // Test state for manual testing
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
void executeRetrieveState();
void executeStoreState();
void executeTestState();

// State reset functions
void resetIdleState();
void resetHomeState();
void resetRetrieveState();
void resetStoreState();
void resetTestState();

// Home state specific functions
void performTestMotionSequence();

// State reference setup functions
void setHomeReferences(FastAccelStepper* motor, Bounce2::Button* homeSwitch);
void setHomeServoReferences(ServoControl* servo, ServoAccelerationController* controller);
void setRetrieveReferences(FastAccelStepper* motor);
void setStoreReferences(FastAccelStepper* motor);
void setTestReferences(FastAccelStepper* motor, ServoAccelerationController* servoController, CylinderControl* cylinder);
void setIdleReferences(ServoAccelerationController* servoController);

// Test state manual mode functions
void parseManualCommand(String command);
void moveToManualHeight(float heightInches);
void moveToManualAngle(int angleDegrees);
void printManualModeHelp();
void printManualModeStatus();
void toggleManualMode();
void checkSerialCommands();

#endif // STATE_MACHINE_H 