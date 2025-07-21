#ifndef LOADER_FORK_STEPPER_H
#define LOADER_FORK_STEPPER_H

#include <Arduino.h>
#include <FastAccelStepper.h>
#include "Config/Config.h"

//* ************************************************************************
//* ************************ LOADER FORK STEPPER **************************
//* ************************************************************************
// Loader fork stepper motor control class
// Replaces pneumatic cylinder with precise stepper motor movement
// Uses 400 steps/rev with 20T 2GT pulley for belt-driven movement

class LoaderForkStepper {
private:
    FastAccelStepper* stepper;    // Pointer to stepper motor object
    int stepPin;                  // Step pin for stepper motor
    int dirPin;                   // Direction pin for stepper motor
    bool isExtended;              // Current state (true = extended, false = retracted)
    int currentPosition;          // Current position in steps (0 = home position)
    
    // Movement constants
    static const int STEPS_PER_REVOLUTION = 400;  // 400 steps per revolution
    static const int PULLEY_TEETH = 20;           // 20T 2GT pulley
    static constexpr float BELT_PITCH_MM = 2.0;       // 2GT belt pitch in mm
    static constexpr float MM_PER_INCH = 25.4;        // Conversion factor
    
    // Movement distance (3 inches forward/backward)
    static const int EXTEND_DISTANCE_STEPS = 3 * 254;  // 3 inches forward (254 steps/inch for 20T 2GT)
    static const int RETRACT_DISTANCE_STEPS = 3 * 254; // 3 inches backward (254 steps/inch for 20T 2GT)
    
public:
    // Constructor
    LoaderForkStepper(int stepPin, int dirPin);
    
    // Initialization
    void begin(FastAccelStepper* stepperObj);
    
    // Movement control functions (replacing cylinder functions)
    void extend();                // Move 3 inches forward (extend)
    void extendToPickPosition();  // Move to pick position using config distance
    void extendToPlacePosition(); // Move to place position using config distance
    void retract();               // Move 3 inches backward (retract)
    void toggle();                // Toggle between extended/retracted states
    
    // Status functions
    bool getState();              // Get current state (true = extended, false = retracted)
    bool isForkExtended();        // Check if currently extended
    bool isMoving();              // Check if motor is currently moving
    int getCurrentPosition();     // Get current position in steps
    
    // Utility functions
    void setCurrentPosition(int position);  // Set current position (for homing)
    void stop();                  // Stop motor movement
    FastAccelStepper* getStepper();        // Get stepper motor object for direct control
};

#endif // LOADER_FORK_STEPPER_H 