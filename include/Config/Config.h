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
//* ************************ HEIGHT ADJUSTMENT SETTINGS *******************
//* ************************************************************************
// Height adjustment settings
extern float HEIGHT_ADJUSTMENT_INCHES; // Height adjustment during sequence (0.5 inches)
extern int HEIGHT_ADJUSTMENT_STEPS;    // Height adjustment in steps

// Idle state settings
extern float IDLE_HEIGHT_INCHES;       // Height for idle state (6.0 inches)
extern int IDLE_HEIGHT_STEPS;          // Idle height in steps
extern float IDLE_ANGLE_DEGREES;       // Servo angle for idle state (half-degree precision)

//* ************************************************************************
//* ************************ SERVO SETTINGS ********************************
//* ************************************************************************
// Servo sequence positions (in degrees)
extern float SERVO_START_POS;    // First position in sequence (90 degrees, half-degree precision)
extern float SERVO_SECOND_POS;   // Second position in sequence (45 degrees, half-degree precision)
extern float SERVO_THIRD_POS;    // Third position in sequence (70 degrees, half-degree precision)

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
//* ************************ LOADING TRAY SETTINGS ***********************
//* ************************************************************************
// Loading tray position and fork extension settings
extern float LOADING_TRAY_HEIGHT_INCHES;    // Loading tray height in inches
extern float LOADING_TRAY_ANGLE_DEGREES;    // Loading tray servo angle in degrees

//* ************************************************************************
//* ************************ LOADING TRAY FORK SETTINGS *******************
//* ************************************************************************
// Loading tray specific fork extension settings
extern float LOADING_TRAY_FORK_PICK_EXTENSION_INCHES;    // Fork extension for picking from loading tray
extern int LOADING_TRAY_FORK_PICK_EXTENSION_STEPS;       // Pick extension in steps
extern float LOADING_TRAY_FORK_PLACE_EXTENSION_INCHES;   // Fork extension for placing to loading tray
extern int LOADING_TRAY_FORK_PLACE_EXTENSION_STEPS;      // Place extension in steps

// Cell specific fork extension settings
extern float CELL_FORK_PICK_EXTENSION_INCHES;            // Fork extension for picking from cells
extern int CELL_FORK_PICK_EXTENSION_STEPS;               // Cell pick extension in steps
extern float CELL_FORK_PLACE_EXTENSION_INCHES;           // Fork extension for placing to cells
extern int CELL_FORK_PLACE_EXTENSION_STEPS;              // Cell place extension in steps

//* ************************************************************************
//* ************************ CELL CONFIGURATION ***************************
//* ************************************************************************
// Include cell configuration for 20 storage cells
#include "Config/CellConfig.h"

#endif 