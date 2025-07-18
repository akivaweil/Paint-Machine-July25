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
extern int SERVO_MOVE_DELAY;      // Wait time between servo movements
extern int HOME_SWITCH_DEBOUNCE;  // Home switch debounce time
extern int START_BUTTON_DEBOUNCE; // Start button debounce time

//* ************************************************************************
//* ************************ Z-AXIS MOTOR SETTINGS ************************
//* ************************************************************************
// Z-axis stepper motor settings
extern float Z_MAX_SPEED;         // Maximum speed in steps per second
extern float Z_ACCELERATION;      // Acceleration in steps per second^2
extern float Z_HOMING_SPEED;      // Homing speed in steps per second

#endif 