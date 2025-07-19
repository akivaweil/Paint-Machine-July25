#include "Config/Pins_Definitions.h"

//* ************************************************************************
//* ************************ PIN DEFINITIONS ******************************
//* ************************************************************************
// Pin assignments for Freenove ESP32-S3 WROOM board
// Paint Machine specific pin configuration

// INPUT PINS (Active HIGH with pulldown)
int START_BUTTON_PIN = 2;         // Start button for cycle operation (active high with pulldown)
int Z_HOME_SWITCH_PIN = 15;        // Z-axis homing limit switch (active high with pulldown)

// OUTPUT PINS - Z-AXIS STEPPER MOTOR
int Z_MOTOR_STEP_PIN = 4;         // Z-axis stepper motor step pin
int Z_MOTOR_DIR_PIN = 5;          // Z-axis stepper motor direction pin

// OUTPUT PINS - SERVO
int LOADER_SERVO_PIN = 41;         // Loader servo control pin

// OUTPUT PINS - PNEUMATIC CYLINDERS
int EXTENSION_CYLINDER_PIN = 14;   // Extension cylinder relay control pin (HIGH = extended, LOW = retracted) 