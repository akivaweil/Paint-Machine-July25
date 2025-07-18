#include "Config/Pins_Definitions.h"

//* ************************************************************************
//* ************************ PIN DEFINITIONS ******************************
//* ************************************************************************
// Pin assignments for Freenove ESP32-S3 WROOM board
// Paint Machine specific pin configuration

// INPUT PINS (Physical switches: Active HIGH with pulldown)
int START_BUTTON_PIN = 2;         // Start loader cycle button (active high with pulldown)
int STOP_BUTTON_PIN = 3;          // Stop/Emergency button (active high with pulldown)

// INPUT PINS (Sensors: Active LOW with pullup)
int Z_HOME_SWITCH_PIN = 6;        // Z-axis homing limit switch (active low with pullup)
int Z_BOTTOM_SWITCH_PIN = 7;      // Z-axis bottom limit switch (active low with pullup)

// OUTPUT PINS - Z-AXIS STEPPER MOTOR
int Z_MOTOR_STEP_PIN = 4;         // Z-axis stepper motor step pin
int Z_MOTOR_DIR_PIN = 5;          // Z-axis stepper motor direction pin
int Z_MOTOR_ENABLE_PIN = 8;       // Z-axis stepper motor enable pin (active low)

// OUTPUT PINS - ACTUATORS
int X_SERVO_PIN = 9;              // X-axis rotation servo control pin
int LOADER_RELAY_PIN = 10;        // Loader mechanism relay control pin (5V relay)
int LED_STATUS_PIN = 11;          // Status LED pin

// OUTPUT PINS - SIGNALS
int READY_SIGNAL_PIN = 12;        // Ready signal output to other machines
int LOADING_SIGNAL_PIN = 13;      // Loading in progress signal 