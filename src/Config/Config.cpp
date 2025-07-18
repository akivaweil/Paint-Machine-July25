#include "Config/Config.h"

//* ************************************************************************
//* ************************* CONFIGURATION *******************************
//* ************************************************************************
// Paint Machine Configuration Settings

//* ************************************************************************
//* ************************ MECHANICAL SETTINGS **************************
//* ************************************************************************
int STEPS_PER_REV = 200;       // Standard stepper motor steps per revolution (1.8° per step)
int PULLEY_TEETH = 20;         // GT2 pulley teeth count
float BELT_PITCH = 2.0;        // GT2 belt pitch in mm
float STEPS_PER_MM = ((float)STEPS_PER_REV / (PULLEY_TEETH * BELT_PITCH));
float STEPS_PER_INCH = (STEPS_PER_MM * 25.4);

//* ************************************************************************
//* ************************ LOADER POSITION SETTINGS *********************
//* ************************************************************************
// Z-axis loader position settings (in inches)
float Z_HOME_INCHES = 0.0;         // Z-axis home position (top)
float Z_LOAD_INCHES = 6.0;         // Z-axis loading position (down 6 inches)
float Z_TRAVEL_INCHES = (Z_LOAD_INCHES - Z_HOME_INCHES);

//* ************************************************************************
//* ************************ CONVERTED POSITIONS ***************************
//* ************************************************************************
// Converted positions (in steps)
int Z_HOME_POS = 0;
int Z_LOAD_POS = (int)(Z_LOAD_INCHES * STEPS_PER_INCH);

//* ************************************************************************
//* ************************ SERVO SETTINGS ********************************
//* ************************************************************************
// X-axis servo settings for rotation (in degrees)
int SERVO_HOME_POS = 90;       // Home rotation position
int SERVO_LOAD_POS = 45;       // Loading rotation position
int SERVO_NEUTRAL_POS = 90;    // Neutral starting position

//* ************************************************************************
//* ************************ TIMING SETTINGS *******************************
//* ************************************************************************
// Timing settings (in milliseconds)
int LOAD_DWELL_TIME = 100;       // Dwell time during loading
int SERVO_MOVE_TIME = 500;       // Wait time for servo movement
int CYCLE_DELAY_TIME = 1000;     // Delay between loader cycles

//* ************************************************************************
//* ************************ Z-AXIS MOTOR SETTINGS ************************
//* ************************************************************************
// Z-axis stepper motor settings
float Z_MAX_SPEED = 1000.0;      // Maximum speed in steps per second
float Z_ACCELERATION = 2000.0;   // Acceleration in steps per second^2
float Z_HOMING_SPEED = 200.0;    // Slow homing speed
float Z_LOAD_SPEED = 500.0;      // Medium speed for loading operations

//* ************************************************************************
//* ************************ LOADER PROCESS SETTINGS **********************
//* ************************************************************************
// Loader process specific settings
int LOAD_CYCLES = 1;             // Number of load cycles per operation
bool AUTO_RETURN_HOME = true;    // Automatically return to home after load 