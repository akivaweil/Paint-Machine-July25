#ifndef CONFIG_H
#define CONFIG_H

//* ************************************************************************
//* ************************ CONFIGURATION SETTINGS ***********************
//* ************************************************************************
//! Configuration settings for the paint machine operation
//! All settings are defined as external variables for easy modification

//* ************************ MECHANICAL SETTINGS **************************
// Mechanical constants for stepper motor calculations
extern int STEPS_PER_REV;              // Stepper motor steps per revolution (1.8° per step = 400)
extern int PULLEY_TEETH;               // GT2 pulley teeth count (20-tooth pulley)
extern float BELT_PITCH;               // GT2 belt pitch in mm (2.0mm)
extern float STEPS_PER_MM;             // Calculated steps per mm
extern float STEPS_PER_INCH;           // Calculated steps per inch

//* ************************ LOADER HEIGHT POSITION SETTINGS *********************
// Loader height position settings (in inches)
extern float Z_HOME_OFFSET_INCHES;    // Distance to move away from home switch after homing
extern int Z_HOME_OFFSET_STEPS;       // Home offset in steps

// Loader height cycle movement settings
extern float Z_CYCLE_DISTANCE_INCHES; // Distance to move down/up during cycle
extern int Z_CYCLE_DISTANCE_STEPS;    // Cycle distance in steps

//* ************************ SERVO SETTINGS *******************************
// Loader servo settings
extern int SERVO_FREQUENCY;            // Servo PWM frequency in Hz (typically 50)
extern int SERVO_MIN_PULSE;            // Minimum pulse width in microseconds
extern int SERVO_MAX_PULSE;            // Maximum pulse width in microseconds
extern int SERVO_MIN_ANGLE;            // Minimum servo angle in degrees
extern int SERVO_MAX_ANGLE;            // Maximum servo angle in degrees

//* ************************ TIMING SETTINGS ******************************
// Button and switch debounce settings
extern int BUTTON_DEBOUNCE;            // Button debounce time in milliseconds
extern int HOME_SWITCH_DEBOUNCE;       // Home switch debounce time in milliseconds

//* ************************ LOADER HEIGHT MOTOR SETTINGS ************************
// Loader height stepper motor settings
extern float Z_MAX_SPEED;         // Maximum speed in steps per second
extern float Z_ACCELERATION;      // Acceleration in steps per second^2
extern float Z_HOMING_SPEED;      // Homing speed in steps per second

#endif 