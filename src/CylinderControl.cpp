#include <Arduino.h>
#include "CylinderControl.h"

//* ************************************************************************
//* ************************ CYLINDER CONTROL ******************************
//* ************************************************************************
// Extension cylinder control implementation
// Handles relay-based pneumatic cylinder operations

CylinderControl::CylinderControl(int pin) {
    cylinderPin = pin;
    isExtended = false;  // Initialize as retracted
}

void CylinderControl::begin() {
    //! ************************************************************************
    //! STEP 1: SETUP CYLINDER RELAY PIN AS OUTPUT
    //! ************************************************************************
    pinMode(cylinderPin, OUTPUT);
    
    //! ************************************************************************
    //! STEP 2: INITIALIZE CYLINDER TO RETRACTED STATE
    //! ************************************************************************
    digitalWrite(cylinderPin, LOW);  // Ensure cylinder starts retracted
    isExtended = false;
}

void CylinderControl::extend() {
    //! ************************************************************************
    //! STEP 1: EXTEND CYLINDER BY SETTING RELAY HIGH
    //! ************************************************************************
    digitalWrite(cylinderPin, HIGH);
    isExtended = true;
}

void CylinderControl::retract() {
    //! ************************************************************************
    //! STEP 1: RETRACT CYLINDER BY SETTING RELAY LOW
    //! ************************************************************************
    digitalWrite(cylinderPin, LOW);
    isExtended = false;
}

void CylinderControl::toggle() {
    //! ************************************************************************
    //! STEP 1: TOGGLE CYLINDER STATE BASED ON CURRENT POSITION
    //! ************************************************************************
    if (isExtended) {
        retract();
    } else {
        extend();
    }
}

bool CylinderControl::getState() {
    // Return current cylinder state
    return isExtended;
}

bool CylinderControl::isCylinderExtended() {
    // Check if cylinder is currently extended
    return isExtended;
} 