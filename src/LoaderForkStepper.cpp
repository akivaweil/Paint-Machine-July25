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
        int targetPosition = currentPosition + FORK_MAX_DISTANCE_STEPS;
        stepper->moveTo(targetPosition);
        currentPosition = targetPosition;
        isExtended = true;
    }
}

void LoaderForkStepper::retract() {
    //! ************************************************************************
    //! STEP 1: MOVE BACKWARD (RETRACT) USING CONFIG DISTANCE
    //! ************************************************************************
    if (stepper && isExtended) {
        int targetPosition = currentPosition - FORK_MAX_DISTANCE_STEPS;
        stepper->moveTo(targetPosition);
        currentPosition = targetPosition;
        isExtended = false;
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