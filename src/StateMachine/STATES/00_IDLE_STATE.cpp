//* ************************************************************************
//* ************************ IDLE STATE ***********************************
//* ************************************************************************
//! IDLE state - Machine waits for commands
//! This is the default state when no operations are running

#include <Arduino.h>
#include "StateMachine.h"
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"


//* ************************************************************************
//* ************************ IDLE STATE VARIABLES *************************
//* ************************************************************************
static bool idleStateInitialized = false;


//* ************************************************************************
//* ************************ IDLE STATE FUNCTIONS *************************
//* ************************************************************************

void executeIdleState() {
    //! ************************************************************************
    //! EXECUTE IDLE STATE - WAITING FOR COMMANDS
    //! ************************************************************************
    
    // Initialize idle state on first entry
    if (!idleStateInitialized) {
        Serial.println("=== ENTERING IDLE STATE ===");
        Serial.println("Machine ready - waiting for commands");
        

        
        idleStateInitialized = true;
    }
    
    //! ************************************************************************
    //! STEP 1: CHECK FOR START BUTTON PRESS
    //! ************************************************************************
    // This will be handled in main loop - idle state just waits
    

    
    //! ************************************************************************
    //! STEP 3: PERFORM IDLE TASKS
    //! ************************************************************************
    // Any background tasks can be performed here
    // For now, just maintain the idle state
    
    //! ************************************************************************
    //! STEP 4: STATE TRANSITIONS
    //! ************************************************************************
    // Transitions to other states will be triggered by external events
    // (button presses, commands, etc.) handled in main loop
}

void resetIdleState() {
    //! ************************************************************************
    //! RESET IDLE STATE FLAGS
    //! ************************************************************************
    idleStateInitialized = false;
}

 