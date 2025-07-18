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
float Z_HOME_OFFSET_INCHES = 5.0;    // Distance to move away from home switch after homing
int Z_HOME_OFFSET_STEPS = (int)(Z_HOME_OFFSET_INCHES * STEPS_PER_INCH);

// Z-axis cycle movement settings
float Z_CYCLE_DISTANCE_INCHES = 2.0;  // Distance to move down/up during cycle (2 inches)
int Z_CYCLE_DISTANCE_STEPS = (int)(Z_CYCLE_DISTANCE_INCHES * STEPS_PER_INCH);

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
int SERVO_MOVE_DELAY = 1000;      // Wait time between servo movements (1 second)
int HOME_SWITCH_DEBOUNCE = 0;      // Home switch debounce time (5ms)
int START_BUTTON_DEBOUNCE = 20;    // Start button debounce time (50ms)
//* ************************************************************************
//* ************************ Z-AXIS MOTOR SETTINGS ************************
//* ************************************************************************
// Z-axis stepper motor settings
float Z_MAX_SPEED = 5000.0;      // Maximum speed in steps per second
float Z_ACCELERATION = 5000.0;   // Acceleration in steps per second^2
float Z_HOMING_SPEED = 400.0;    // Slow homing speed 