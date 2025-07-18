#ifndef PINS_DEFINITIONS_H
#define PINS_DEFINITIONS_H

//* ************************************************************************
//* ************************ PIN DEFINITIONS ******************************
//* ************************************************************************
// Pin assignments for Freenove ESP32-S3 WROOM board
// Paint Machine specific pin configuration

// INPUT PINS
extern int START_BUTTON_PIN;       // Start button for cycle operation (active high with pulldown)
extern int Z_HOME_SWITCH_PIN;      // Z-axis homing limit switch (active high with pulldown)

// OUTPUT PINS - Z-AXIS STEPPER MOTOR
extern int Z_MOTOR_STEP_PIN;       // Z-axis stepper motor step pin
extern int Z_MOTOR_DIR_PIN;        // Z-axis stepper motor direction pin

// OUTPUT PINS - SERVO
extern int LOADER_SERVO_PIN;       // Loader servo control pin

#endif 