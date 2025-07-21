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
float PICK_HEIGHT_INCHES = 1.0;   // Height to move to for pick operation (3.2 inches)
int PICK_HEIGHT_STEPS = (int)(PICK_HEIGHT_INCHES * STEPS_PER_INCH);
int PICK_ANGLE_DEGREES = 174;      // Servo angle for pick operation (21.5 degrees)

// Place state settings
float PLACE_HEIGHT_INCHES = 5.4;      // Height to move to for place operation (8 inches)
int PLACE_HEIGHT_STEPS = (int)(PLACE_HEIGHT_INCHES * STEPS_PER_INCH);
int PLACE_ANGLE_DEGREES = 102;         // Servo angle for place operation (91 degrees)

// Height adjustment settings
float HEIGHT_ADJUSTMENT_INCHES = 0.7; // Height adjustment during sequence (.7 inches)
int HEIGHT_ADJUSTMENT_STEPS = (int)(HEIGHT_ADJUSTMENT_INCHES * STEPS_PER_INCH);

// Idle state settings
float IDLE_HEIGHT_INCHES = 2.0;        // Height for idle state (5 inches)
int IDLE_HEIGHT_STEPS = (int)(IDLE_HEIGHT_INCHES * STEPS_PER_INCH);
int IDLE_ANGLE_DEGREES = 174;           // Servo angle for idle state (30 degrees)

//* ************************************************************************
//* ************************ SERVO SETTINGS ********************************
//* ************************************************************************
// Servo sequence positions (in degrees)
int SERVO_START_POS = 90;      // First position in sequence
int SERVO_SECOND_POS = 45;     // Second position in sequence  
int SERVO_THIRD_POS = 70;      // Third position in sequence

//* ************************************************************************
//* ************************ TIMING SETTINGS *******************************
//* ************************************************************************
// Timing settings (in milliseconds)
int SERVO_MOVE_DELAY = 500;      // Wait time between servo movements (1 second)
int HOME_SWITCH_DEBOUNCE = 5;      // Home switch debounce time (5ms)
int START_BUTTON_DEBOUNCE = 20;    // Start button debounce time (50ms)

// Sequence timing settings
int CYLINDER_EXTEND_WAIT = 750;     // Wait time after extending cylinder (750ms)
int HEIGHT_ADJUST_WAIT = 100;       // Wait time after height adjustment (100ms)
int CYLINDER_RETRACT_WAIT = 750;   // Wait time after retracting cylinder (1000ms)

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
float FORK_MAX_DISTANCE_INCHES = 3.5;   // Maximum travel distance for fork (inches) - increased from 3.0
int FORK_MAX_DISTANCE_STEPS = (int)(FORK_MAX_DISTANCE_INCHES * 254);  // 254 steps per inch for 20T 2GT belt
float FORK_STEPS_PER_INCH = 254.0;      // Steps per inch for fork movement (20T 2GT belt)
float FORK_MAX_SPEED = 5000.0;          // Maximum speed for fork movement (steps/sec)
float FORK_ACCELERATION = 10000.0;      // Acceleration for fork movement (steps/sec²) 