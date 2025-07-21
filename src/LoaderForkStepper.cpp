#include <Arduino.h>
#include "LoaderForkStepper.h"
#include "Config/Config.h"

//* ************************************************************************
//* ************************ LOADER FORK STEPPER **************************
//* ************************************************************************
// Loader fork stepper motor control implementation
// Replaces pneumatic cylinder with precise stepper motor movement

LoaderForkStepper::LoaderForkStepper(int stepPin, int dirPin) {
    this->stepPin = stepPin;
    this->dirPin = dirPin;
    this->stepper = NULL;
    this->isExtended = false;     // Initialize as retracted
    this->currentPosition = 0;    // Start at position 0
    this->lastExtensionDistance = 0; // Initialize extension distance tracking
}

void LoaderForkStepper::begin(FastAccelStepper* stepperObj) {
    //! ************************************************************************
    //! STEP 1: STORE STEPPER OBJECT REFERENCE
    //! ************************************************************************
    stepper = stepperObj;
    
    //! ************************************************************************
    //! STEP 2: CONFIGURE STEPPER MOTOR PINS
    //! ************************************************************************
    if (stepper) {
        // Configure motor settings using config values
        stepper->setSpeedInHz(FORK_MAX_SPEED);        // Use config speed
        stepper->setAcceleration(FORK_ACCELERATION);  // Use config acceleration
        
        // Set current position to 0 (assume we're at home)
        stepper->setCurrentPosition(0);
        currentPosition = 0;
    }
    
    //! ************************************************************************
    //! STEP 3: INITIALIZE TO RETRACTED STATE
    //! ************************************************************************
    isExtended = false;
}

void LoaderForkStepper::extend() {
    //! ************************************************************************
    //! STEP 1: MOVE FORWARD (EXTEND) USING CONFIG DISTANCE
    //! ************************************************************************
    if (stepper && !isExtended) {
        lastExtensionDistance = FORK_MAX_DISTANCE_STEPS;
        int targetPosition = currentPosition + lastExtensionDistance;
        stepper->moveTo(targetPosition);
        currentPosition = targetPosition;
        isExtended = true;
        Serial.println("Fork extending: " + String(FORK_MAX_DISTANCE_INCHES) + " inches (" + String(lastExtensionDistance) + " steps)");
    }
}

void LoaderForkStepper::extendToPickPosition() {
    //! ************************************************************************
    //! STEP 1: MOVE FORWARD (EXTEND) USING PICK CONFIG DISTANCE
    //! ************************************************************************
    if (stepper) {
        lastExtensionDistance = PICK_FORK_EXTENSION_STEPS;
        int targetPosition = currentPosition + lastExtensionDistance;
        stepper->moveTo(targetPosition);
        currentPosition = targetPosition;
        isExtended = true;
        Serial.println("Fork extending to pick position: " + String(PICK_FORK_EXTENSION_INCHES) + " inches (" + String(lastExtensionDistance) + " steps)");
    }
}

void LoaderForkStepper::extendToPlacePosition() {
    //! ************************************************************************
    //! STEP 1: MOVE FORWARD (EXTEND) USING PLACE CONFIG DISTANCE
    //! ************************************************************************
    if (stepper) {
        lastExtensionDistance = PLACE_FORK_EXTENSION_STEPS;
        int targetPosition = currentPosition + lastExtensionDistance;
        stepper->moveTo(targetPosition);
        currentPosition = targetPosition;
        isExtended = true;
        Serial.println("Fork extending to place position: " + String(PLACE_FORK_EXTENSION_INCHES) + " inches (" + String(lastExtensionDistance) + " steps)");
    }
}

void LoaderForkStepper::retract() {
    //! ************************************************************************
    //! STEP 1: MOVE BACKWARD (RETRACT) USING ACTUAL EXTENSION DISTANCE
    //! ************************************************************************
    if (stepper && isExtended) {
        int targetPosition = currentPosition - lastExtensionDistance;
        stepper->moveTo(targetPosition);
        currentPosition = targetPosition;
        isExtended = false;
        float retractInches = (float)lastExtensionDistance / FORK_STEPS_PER_INCH;
        Serial.println("Fork retracting: " + String(retractInches, 2) + " inches (" + String(lastExtensionDistance) + " steps)");
        lastExtensionDistance = 0; // Reset for next operation
    }
}

void LoaderForkStepper::toggle() {
    //! ************************************************************************
    //! STEP 1: TOGGLE BETWEEN EXTENDED AND RETRACTED STATES
    //! ************************************************************************
    if (isExtended) {
        retract();
    } else {
        extend();
    }
}

bool LoaderForkStepper::getState() {
    // Return current state (true = extended, false = retracted)
    return isExtended;
}

bool LoaderForkStepper::isForkExtended() {
    // Check if currently extended
    return isExtended;
}

bool LoaderForkStepper::isMoving() {
    // Check if motor is currently moving
    return stepper ? stepper->isRunning() : false;
}

int LoaderForkStepper::getCurrentPosition() {
    // Get current position in steps
    return currentPosition;
}

void LoaderForkStepper::setCurrentPosition(int position) {
    //! ************************************************************************
    //! STEP 1: SET CURRENT POSITION (FOR HOMING)
    //! ************************************************************************
    currentPosition = position;
    if (stepper) {
        stepper->setCurrentPosition(position);
    }
}

void LoaderForkStepper::stop() {
    //! ************************************************************************
    //! STEP 1: STOP MOTOR MOVEMENT
    //! ************************************************************************
    if (stepper) {
        stepper->stopMove();
    }
}

FastAccelStepper* LoaderForkStepper::getStepper() {
    //! ************************************************************************
    //! STEP 1: RETURN STEPPER MOTOR OBJECT FOR DIRECT CONTROL
    //! ************************************************************************
    return stepper;
} 

void LoaderForkStepper::homeFork() {
    //* ************************************************************************
    //* ************************ FORK HOMING **********************************
    //* ************************************************************************
    //! Block until fork is homed using home switch (active HIGH, input pulldown)
    Serial.println("Starting fork homing routine...");
    
    // Set speed and acceleration for safe homing
    if (stepper) {
        stepper->setSpeedInHz(FORK_MAX_SPEED / 10); // Slow speed for homing
        stepper->setAcceleration(FORK_ACCELERATION / 10);
    }

    // Move fork toward home until switch is triggered
    if (stepper) {
        stepper->runBackward();
        Serial.println("Fork moving toward home switch...");
        unsigned long startTime = millis();
        while (!homeSwitch || homeSwitch->read() == LOW) { // Wait for active HIGH
            // Block until switch is triggered
            if (homeSwitch) {
                homeSwitch->update(); // Update switch state for proper debouncing
            }
            // Debug output every 500ms
            if (millis() - startTime > 500) {
                Serial.println("Fork homing in progress - Switch state: " + String(homeSwitch ? (homeSwitch->read() ? "HIGH" : "LOW") : "NULL"));
                startTime = millis();
            }
            delay(1); // Small delay to avoid busy-waiting
        }
        stepper->forceStop();
        stepper->setCurrentPosition(0);
        currentPosition = 0;
        isExtended = false;
        Serial.println("Fork home switch triggered - Fork homed, position set to 0");
    }
} 

void LoaderForkStepper::setHomeSwitch(Bounce2::Button* homeSwitchObj) {
    //! ************************************************************************
    //! SET THE DEBOUNCED HOME SWITCH OBJECT
    //! ************************************************************************
    homeSwitch = homeSwitchObj;
} 