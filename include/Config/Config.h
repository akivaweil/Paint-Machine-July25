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
//* ************************ RETRIEVE/STORE SEQUENCE SETTINGS *************
//* ************************************************************************
// Retrieve state settings
extern float RETRIEVE_HEIGHT_INCHES;  // Height to move to for retrieve operation
extern int RETRIEVE_HEIGHT_STEPS;     // Retrieve height in steps
extern int RETRIEVE_ANGLE_DEGREES;    // Servo angle for retrieve operation

// Store state settings  
extern float STORE_HEIGHT_INCHES;     // Height to move to for store operation
extern int STORE_HEIGHT_STEPS;        // Store height in steps
extern int STORE_ANGLE_DEGREES;       // Servo angle for store operation

// Height adjustment settings
extern float HEIGHT_ADJUSTMENT_INCHES; // Height adjustment during sequence (.4 inches)
extern int HEIGHT_ADJUSTMENT_STEPS;    // Height adjustment in steps

// Idle state settings
extern float IDLE_HEIGHT_INCHES;       // Height for idle state (5 inches)
extern int IDLE_HEIGHT_STEPS;          // Idle height in steps
extern int IDLE_ANGLE_DEGREES;         // Servo angle for idle state (30 degrees)

//* ************************************************************************
//* ************************ SERVO SETTINGS ********************************
//* ************************************************************************
// Servo sequence positions (in degrees)
extern int SERVO_START_POS;      // First position in sequence (90 degrees)
extern int SERVO_SECOND_POS;     // Second position in sequence (45 degrees)
extern int SERVO_THIRD_POS;      // Third position in sequence (70 degrees)

// Servo jiggle settings for wood positioning
extern int SERVO_JIGGLE_ANGLE;   // Jiggle angle in degrees (5 degrees)
extern int SERVO_JIGGLE_DELAY;   // Delay between jiggle movements (50ms)

//* ************************************************************************
//* ************************ TIMING SETTINGS *******************************
//* ************************************************************************
// Timing settings (in milliseconds)
extern int SERVO_MOVE_DELAY;      // Wait time between servo movements
extern int HOME_SWITCH_DEBOUNCE;  // Home switch debounce time
extern int START_BUTTON_DEBOUNCE; // Start button debounce time

// Sequence timing settings
extern int CYLINDER_EXTEND_WAIT;  // Wait time after extending cylinder (750ms)
extern int HEIGHT_ADJUST_WAIT;    // Wait time after height adjustment (100ms)
extern int CYLINDER_RETRACT_WAIT; // Wait time after retracting cylinder (1000ms)

//* ************************************************************************
//* ************************ Z-AXIS MOTOR SETTINGS ************************
//* ************************************************************************
// Z-axis stepper motor settings
extern float Z_MAX_SPEED;         // Maximum speed in steps per second
extern float Z_ACCELERATION;      // Acceleration in steps per second^2
extern float Z_HOMING_SPEED;      // Homing speed in steps per second

#endif 