#ifndef CONFIG_H
#define CONFIG_H

//* ************************************************************************
//* ************************* CONFIGURATION *******************************
//* ************************************************************************
// Paint Machine Configuration Settings

//* ************************************************************************
//* ************************ MECHANICAL SETTINGS **************************
//* ************************************************************************
extern int STEPS_PER_REV;     // Steps per revolution
extern int PULLEY_TEETH;      // Number of teeth on pulleys
extern float BELT_PITCH;      // GT2 belt pitch in mm
extern float STEPS_PER_MM;
extern float STEPS_PER_INCH;

//* ************************************************************************
//* ************************ Z-AXIS POSITION SETTINGS *********************
//* ************************************************************************
// Z-axis position settings
extern float Z_HOME_OFFSET_INCHES;    // Distance to move away from home switch after homing
extern int Z_HOME_OFFSET_STEPS;       // Home offset in steps

// Z-axis cycle movement settings
extern float Z_CYCLE_DISTANCE_INCHES; // Distance to move down/up during cycle
extern int Z_CYCLE_DISTANCE_STEPS;    // Cycle distance in steps

//* ************************************************************************
//* ************************ PICK/PLACE SEQUENCE SETTINGS *****************
//* ************************************************************************
// Pick state settings
extern float PICK_HEIGHT_INCHES;  // Height to move to for pick operation
extern int PICK_HEIGHT_STEPS;     // Pick height in steps
extern int PICK_ANGLE_DEGREES;    // Servo angle for pick operation
extern float PICK_FORK_EXTENSION_INCHES;  // Fork extension distance for pick operation
extern int PICK_FORK_EXTENSION_STEPS;     // Pick fork extension in steps

// Place state settings  
extern float PLACE_HEIGHT_INCHES;     // Height to move to for place operation
extern int PLACE_HEIGHT_STEPS;        // Place height in steps
extern int PLACE_ANGLE_DEGREES;       // Servo angle for place operation
extern float PLACE_FORK_EXTENSION_INCHES;  // Fork extension distance for place operation
extern int PLACE_FORK_EXTENSION_STEPS;     // Place fork extension in steps

// Height adjustment settings
extern float HEIGHT_ADJUSTMENT_INCHES; // Height adjustment during sequence (.4 inches)
extern int HEIGHT_ADJUSTMENT_STEPS;    // Height adjustment in steps

// Idle state settings
extern float IDLE_HEIGHT_INCHES;       // Height for idle state (5 inches)
extern int IDLE_HEIGHT_STEPS;          // Idle height in steps
extern int IDLE_ANGLE_DEGREES;         // Servo angle for idle state (30 degrees)

//* ************************************************************************
//* ************************ SERVO SETTINGS ********************************
//* ************************************************************************
// Servo sequence positions (in degrees)
extern int SERVO_START_POS;      // First position in sequence (90 degrees)
extern int SERVO_SECOND_POS;     // Second position in sequence (45 degrees)
extern int SERVO_THIRD_POS;      // Third position in sequence (70 degrees)

//* ************************************************************************
//* ************************ TIMING SETTINGS *******************************
//* ************************************************************************
// Timing settings (in milliseconds)
extern int HOME_SWITCH_DEBOUNCE;  // Home switch debounce time
extern int START_BUTTON_DEBOUNCE; // Start button debounce time
extern int FORK_HOME_SWITCH_DEBOUNCE;

// Note: All sequence timing is now based on actual completion of operations

//* ************************************************************************
//* ************************ Z-AXIS MOTOR SETTINGS ************************
//* ************************************************************************
// Z-axis stepper motor settings
extern float Z_MAX_SPEED;         // Maximum speed in steps per second
extern float Z_ACCELERATION;      // Acceleration in steps per second^2
extern float Z_HOMING_SPEED;      // Homing speed in steps per second

//* ************************************************************************
//* ************************ FORK SETTINGS ********************************
//* ************************************************************************
// Loader fork stepper motor settings
extern float FORK_MAX_DISTANCE_INCHES;  // Maximum travel distance for fork (inches)
extern int FORK_MAX_DISTANCE_STEPS;     // Maximum travel distance in steps
extern float FORK_STEPS_PER_INCH;       // Steps per inch for fork movement
extern float FORK_MAX_SPEED;            // Maximum speed for fork movement (steps/sec)
extern float FORK_ACCELERATION;         // Acceleration for fork movement (steps/sec²)

//* ************************************************************************
//* ************************ SLOT CONFIGURATION ***************************
//* ************************************************************************
// Include slot configuration for 20 storage slots
#include "Config/SlotConfig.h"

#endif 