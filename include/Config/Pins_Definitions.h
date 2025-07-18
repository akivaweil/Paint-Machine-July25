#ifndef PINS_DEFINITIONS_H
#define PINS_DEFINITIONS_H

//* ************************************************************************
//* ************************ PIN DEFINITIONS ******************************
//* ************************************************************************
// Pin assignments for Freenove ESP32-S3 WROOM board
// Paint Machine specific pin configuration

// INPUT PINS
extern int START_BUTTON_PIN;       // Start loader cycle button (active high with pulldown)
extern int STOP_BUTTON_PIN;        // Stop/Emergency button (active high with pulldown)
extern int Z_HOME_SWITCH_PIN;      // Z-axis homing limit switch (active low with pullup)
extern int Z_BOTTOM_SWITCH_PIN;    // Z-axis bottom limit switch (active low with pullup)

// OUTPUT PINS - Z-AXIS STEPPER MOTOR
extern int Z_MOTOR_STEP_PIN;       // Z-axis stepper motor step pin
extern int Z_MOTOR_DIR_PIN;        // Z-axis stepper motor direction pin
extern int Z_MOTOR_ENABLE_PIN;     // Z-axis stepper motor enable pin (active low)

// OUTPUT PINS - ACTUATORS
extern int X_SERVO_PIN;            // X-axis rotation servo control pin
extern int LOADER_RELAY_PIN;       // Loader mechanism relay control pin
extern int LED_STATUS_PIN;         // Status LED pin

// OUTPUT PINS - SIGNALS
extern int READY_SIGNAL_PIN;       // Ready signal output to other machines
extern int LOADING_SIGNAL_PIN;     // Loading in progress signal

#endif 