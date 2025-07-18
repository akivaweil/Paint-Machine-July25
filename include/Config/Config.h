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
//* ************************ LOADER POSITION SETTINGS *********************
//* ************************************************************************
// Z-axis loader position settings (vertical movement)
extern float Z_HOME_INCHES;          // Z-axis home position
extern float Z_LOAD_INCHES;          // Z-axis loading position
extern float Z_TRAVEL_INCHES;        // Total Z travel distance

//* ************************************************************************
//* ************************ CONVERTED POSITIONS ***************************
//* ************************************************************************
// Converted positions (in steps)
extern int Z_HOME_POS;
extern int Z_LOAD_POS;

//* ************************************************************************
//* ************************ SERVO SETTINGS ********************************
//* ************************************************************************
// X-axis servo settings for rotation (in degrees)
extern int SERVO_HOME_POS;       // Home rotation position
extern int SERVO_LOAD_POS;       // Loading rotation position
extern int SERVO_NEUTRAL_POS;    // Neutral position

//* ************************************************************************
//* ************************ TIMING SETTINGS *******************************
//* ************************************************************************
// Timing settings (in milliseconds)
extern int LOAD_DWELL_TIME;      // Dwell time during loading
extern int SERVO_MOVE_TIME;      // Wait time for servo movement
extern int CYCLE_DELAY_TIME;     // Delay between loader cycles

//* ************************************************************************
//* ************************ Z-AXIS MOTOR SETTINGS ************************
//* ************************************************************************
// Z-axis stepper motor settings
extern float Z_MAX_SPEED;         // Maximum speed in steps per second
extern float Z_ACCELERATION;      // Acceleration in steps per second^2
extern float Z_HOMING_SPEED;      // Homing speed in steps per second
extern float Z_LOAD_SPEED;        // Loading speed in steps per second

//* ************************************************************************
//* ************************ LOADER PROCESS SETTINGS **********************
//* ************************************************************************
// Loader process specific settings
extern int LOAD_CYCLES;           // Number of load cycles
extern bool AUTO_RETURN_HOME;     // Automatically return to home after load

#endif 