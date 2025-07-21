#include "Config/Config.h"

//* ************************************************************************
//* ************************* CONFIGURATION *******************************
//* ************************************************************************
// Paint Machine Configuration Settings

//* ************************************************************************
//* ************************ MECHANICAL SETTINGS **************************
//* ************************************************************************
int STEPS_PER_REV = 400;       // Standard stepper motor steps per revolution (1.8° per step)
int PULLEY_TEETH = 20;         // GT2 pulley teeth count (20-tooth pulley)
float BELT_PITCH = 2.0;        // GT2 belt pitch in mm
float STEPS_PER_MM = ((float)STEPS_PER_REV / (PULLEY_TEETH * BELT_PITCH));
float STEPS_PER_INCH = (STEPS_PER_MM * 25.4);

//* ************************************************************************
//* ************************ Z-AXIS POSITION SETTINGS *********************
//* ************************************************************************
// Z-axis position settings (in inches)
float Z_HOME_OFFSET_INCHES = 1.0;    // Distance to move away from home switch after homing
int Z_HOME_OFFSET_STEPS = (int)(Z_HOME_OFFSET_INCHES * STEPS_PER_INCH);

// Z-axis cycle movement settings
float Z_CYCLE_DISTANCE_INCHES = 2.0;  // Distance to move down/up during cycle (2 inches)
int Z_CYCLE_DISTANCE_STEPS = (int)(Z_CYCLE_DISTANCE_INCHES * STEPS_PER_INCH);

//* ************************************************************************
//* ************************ PICK/PLACE SEQUENCE SETTINGS *****************
//* ************************************************************************
// Pick state settings
float PICK_HEIGHT_INCHES = 0.8;   // Height to move to for pick operation (3.2 inches)
int PICK_HEIGHT_STEPS = (int)(PICK_HEIGHT_INCHES * STEPS_PER_INCH);
float PICK_ANGLE_DEGREES = 126.5;      // Servo angle for pick operation (half-degree precision)
float PICK_FORK_EXTENSION_INCHES = 12.1;   // Fork extension distance for pick operation (inches)
int PICK_FORK_EXTENSION_STEPS = (int)(PICK_FORK_EXTENSION_INCHES * FORK_STEPS_PER_INCH);

// Place state settings
float PLACE_HEIGHT_INCHES = 5.4;      // Height to move to for place operation (8 inches)
int PLACE_HEIGHT_STEPS = (int)(PLACE_HEIGHT_INCHES * STEPS_PER_INCH);
float PLACE_ANGLE_DEGREES = 103.0;         // Servo angle for place operation (half-degree precision)
float PLACE_FORK_EXTENSION_INCHES = 3.8;   // Fork extension distance for place operation (inches)
int PLACE_FORK_EXTENSION_STEPS = (int)(PLACE_FORK_EXTENSION_INCHES * FORK_STEPS_PER_INCH);

// Height adjustment settings
float HEIGHT_ADJUSTMENT_INCHES = 0.7; // Height adjustment during sequence (.7 inches)
int HEIGHT_ADJUSTMENT_STEPS = (int)(HEIGHT_ADJUSTMENT_INCHES * STEPS_PER_INCH);

// Idle state settings
float IDLE_HEIGHT_INCHES = 2.0;        // Height for idle state (5 inches)
int IDLE_HEIGHT_STEPS = (int)(IDLE_HEIGHT_INCHES * STEPS_PER_INCH);
float IDLE_ANGLE_DEGREES = 173.0;           // Servo angle for idle state (half-degree precision)

//* ************************************************************************
//* ************************ SERVO SETTINGS ********************************
//* ************************************************************************
// Servo sequence positions (in degrees)
float SERVO_START_POS = 90.0;      // First position in sequence (half-degree precision)
float SERVO_SECOND_POS = 45.0;     // Second position in sequence (half-degree precision)
float SERVO_THIRD_POS = 70.0;      // Third position in sequence (half-degree precision)

//* ************************************************************************
//* ************************ TIMING SETTINGS *******************************
//* ************************************************************************
// Timing settings (in milliseconds)
int HOME_SWITCH_DEBOUNCE = 5;      // Home switch debounce time (5ms)
int START_BUTTON_DEBOUNCE = 20;    // Start button debounce time (50ms)
int FORK_HOME_SWITCH_DEBOUNCE = 2; // Fork home switch debounce time (5ms)

// Note: All sequence timing is now based on actual completion of operations
// No artificial delays - system responds immediately when operations complete
//* ************************************************************************
//* ************************ Z-AXIS MOTOR SETTINGS ************************
//* ************************************************************************
// Z-axis stepper motor settings
float Z_MAX_SPEED = 8000.0;      // Maximum speed in steps per second
float Z_ACCELERATION = 30000.0;   // Acceleration in steps per second^2
float Z_HOMING_SPEED = 1000.0;    // Slow homing speed

//* ************************************************************************
//* ************************ FORK SETTINGS ********************************
//* ************************************************************************
// Loader fork stepper motor settings
float FORK_MAX_DISTANCE_INCHES = 3.7;   // Maximum travel distance for fork (inches) - increased from 3.0
int FORK_MAX_DISTANCE_STEPS = (int)(FORK_MAX_DISTANCE_INCHES * 254);  // 254 steps per inch for 20T 2GT belt
float FORK_STEPS_PER_INCH = 254.0;      // Steps per inch for fork movement (20T 2GT belt)
float FORK_MAX_SPEED = 10000.0;          // Maximum speed for fork movement (steps/sec)
float FORK_ACCELERATION = 40000.0;      // Acceleration for fork movement (steps/sec²) 