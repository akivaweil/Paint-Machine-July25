#ifndef PINS_DEFINITIONS_H
#define PINS_DEFINITIONS_H

//* ************************************************************************
//* ************************ PIN DEFINITIONS ******************************
//* ************************************************************************
//! Pin definitions for the paint machine hardware components
//! All pins are defined as external variables for easy configuration

// INPUT PINS - SWITCHES AND SENSORS
extern int START_BUTTON_PIN;           // Manual start button (active high with pulldown)
extern int Z_HOME_SWITCH_PIN;      // Loader height homing limit switch (active high with pulldown)

// OUTPUT PINS - LOADER HEIGHT STEPPER MOTOR
extern int Z_MOTOR_STEP_PIN;       // Loader height stepper motor step pin
extern int Z_MOTOR_DIR_PIN;        // Loader height stepper motor direction pin

// OUTPUT PINS - SERVO MOTORS
extern int LOADER_SERVO_PIN;       // Loader servo motor control pin

// OUTPUT PINS - PNEUMATIC CYLINDERS
extern int EXTENSION_CYLINDER_PIN; // Extension cylinder solenoid control pin

#endif 