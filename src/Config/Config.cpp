#include "Config/Config.h"

//* ************************************************************************
//* ************************ CONFIGURATION SETTINGS ***********************
//* ************************************************************************
//! Configuration settings for the paint machine operation
//! All settings are defined as external variables for easy modification

//* ************************ MECHANICAL SETTINGS **************************
// Mechanical constants for stepper motor calculations
int STEPS_PER_REV = 400;               // Stepper motor steps per revolution (1.8° per step)
int PULLEY_TEETH = 20;                 // GT2 pulley teeth count (20-tooth pulley)
float BELT_PITCH = 2.0;                // GT2 belt pitch in mm
float STEPS_PER_MM = ((float)STEPS_PER_REV / (PULLEY_TEETH * BELT_PITCH));
float STEPS_PER_INCH = (STEPS_PER_MM * 25.4);

//* ************************ LOADER HEIGHT POSITION SETTINGS *********************
// Loader height position settings (in inches)
float Z_HOME_OFFSET_INCHES = 1.0;    // Distance to move away from home switch after homing
int Z_HOME_OFFSET_STEPS = (int)(Z_HOME_OFFSET_INCHES * STEPS_PER_INCH);

// Loader height cycle movement settings
float Z_CYCLE_DISTANCE_INCHES = 2.0;  // Distance to move down/up during cycle (2 inches)
int Z_CYCLE_DISTANCE_STEPS = (int)(Z_CYCLE_DISTANCE_INCHES * STEPS_PER_INCH);

//* ************************ SERVO SETTINGS *******************************
// Loader servo settings
int SERVO_FREQUENCY = 50;              // Servo PWM frequency in Hz (typically 50)
int SERVO_MIN_PULSE = 500;             // Minimum pulse width in microseconds
int SERVO_MAX_PULSE = 2500;            // Maximum pulse width in microseconds
int SERVO_MIN_ANGLE = 0;               // Minimum servo angle in degrees
int SERVO_MAX_ANGLE = 180;             // Maximum servo angle in degrees

// Servo sequence positions (in degrees)
int SERVO_START_POS = 90;              // First position in sequence
int SERVO_SECOND_POS = 45;             // Second position in sequence
int SERVO_THIRD_POS = 70;              // Third position in sequence
unsigned long SERVO_MOVE_DELAY = 1000; // Servo movement delay (ms)

//* ************************ TIMING SETTINGS ******************************
// Button and switch debounce settings
int BUTTON_DEBOUNCE = 50;              // Button debounce time in milliseconds
int START_BUTTON_DEBOUNCE = 50;        // Start button debounce time in milliseconds
int HOME_SWITCH_DEBOUNCE = 5;         // Home switch debounce time in milliseconds

//* ************************ LOADER HEIGHT MOTOR SETTINGS ************************
// Loader height stepper motor settings
float Z_MAX_SPEED = 5000.0;      // Maximum speed in steps per second
float Z_ACCELERATION = 5000.0;   // Acceleration in steps per second^2
float Z_HOMING_SPEED = 2000.0;    // Slow homing speed 