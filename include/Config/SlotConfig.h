#ifndef SLOT_CONFIG_H
#define SLOT_CONFIG_H

//* ************************************************************************
//* ************************ SLOT CONFIGURATION ***************************
//* ************************************************************************
// Storage slot configuration for 20 slots with height and servo angle settings

//* ************************************************************************
//* ************************ SLOT STRUCTURE *******************************
//* ************************************************************************
struct SlotPosition {
    float height_inches;    // Height position in inches
    int servo_angle;        // Servo angle in degrees
    bool is_configured;     // Whether this slot has been configured
};

//* ************************************************************************
//* ************************ FUNCTION DECLARATIONS ***********************
//* ************************************************************************
void initializeSlotConfig();
void setSlotPosition(int slot_number, float height_inches, int servo_angle);
SlotPosition getSlotPosition(int slot_number);
int getSlotHeightSteps(int slot_number);
void resetSlotConfig();
void printSlotConfig();

//* ************************************************************************
//* ************************ CONSTANTS ***********************************
//* ************************************************************************
#define TOTAL_SLOTS 20
#define DEFAULT_HEIGHT_INCHES 5.0
#define DEFAULT_SERVO_ANGLE 90

#endif // SLOT_CONFIG_H 