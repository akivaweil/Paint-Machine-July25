#include "Config/Pins_Definitions.h"

//* ************************************************************************
//* ************************ PIN DEFINITIONS ******************************
//* ************************************************************************
//! Pin definitions for the paint machine hardware components
//! All pins are defined as external variables for easy configuration

// INPUT PINS - SWITCHES AND SENSORS
int START_BUTTON_PIN = 0;              // Manual start button (active high with pulldown)
int Z_HOME_SWITCH_PIN = 15;        // Loader height homing limit switch (active high with pulldown)

// OUTPUT PINS - LOADER HEIGHT STEPPER MOTOR
int Z_MOTOR_STEP_PIN = 4;         // Loader height stepper motor step pin
int Z_MOTOR_DIR_PIN = 5;          // Loader height stepper motor direction pin

// OUTPUT PINS - SERVO MOTORS
int LOADER_SERVO_PIN = 6;         // Loader servo motor control pin

// OUTPUT PINS - PNEUMATIC CYLINDERS
int EXTENSION_CYLINDER_PIN = 7;   // Extension cylinder solenoid control pin 