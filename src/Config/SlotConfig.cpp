#include <Arduino.h>
#include "Config/SlotConfig.h"
#include "Config/Config.h"

//* ************************************************************************
//* ************************ SLOT CONFIGURATION ***************************
//* ************************************************************************
// Storage slot configuration for 20 slots with height and servo angle settings

//* ************************************************************************
//* ************************ GLOBAL VARIABLES *****************************
//* ************************************************************************
static SlotPosition slot_positions[TOTAL_SLOTS];

//* ************************************************************************
//* ************************ SLOT POSITION SETTINGS **********************
//* ************************************************************************
// Easy-to-read slot configuration - modify these values as needed
// Format: {height_inches, servo_angle}
// Slot numbers are 0-19 (20 total slots)

//! ************************************************************************
//! SLOT CONFIGURATION - MODIFY THESE VALUES FOR EACH SLOT
//! ************************************************************************
static const float SLOT_HEIGHTS[TOTAL_SLOTS] = {
    3.0,  // Slot 0
    3.2,  // Slot 1
    3.4,  // Slot 2
    3.6,  // Slot 3
    3.8,  // Slot 4
    4.0,  // Slot 5
    4.2,  // Slot 6
    4.4,  // Slot 7
    4.6,  // Slot 8
    4.8,  // Slot 9
    5.0,  // Slot 10
    5.2,  // Slot 11
    5.4,  // Slot 12
    5.6,  // Slot 13
    5.8,  // Slot 14
    6.0,  // Slot 15
    6.2,  // Slot 16
    6.4,  // Slot 17
    6.6,  // Slot 18
    6.8   // Slot 19
};

static const int SLOT_ANGLES[TOTAL_SLOTS] = {
    45,   // Slot 0
    47,   // Slot 1
    49,   // Slot 2
    51,   // Slot 3
    53,   // Slot 4
    55,   // Slot 5
    57,   // Slot 6
    59,   // Slot 7
    61,   // Slot 8
    63,   // Slot 9
    65,   // Slot 10
    67,   // Slot 11
    69,   // Slot 12
    71,   // Slot 13
    73,   // Slot 14
    75,   // Slot 15
    77,   // Slot 16
    79,   // Slot 17
    81,   // Slot 18
    83    // Slot 19
};

//* ************************************************************************
//* ************************ INITIALIZATION *******************************
//* ************************************************************************
void initializeSlotConfig() {
    // Initialize all slots with default values
    for (int i = 0; i < TOTAL_SLOTS; i++) {
        slot_positions[i].height_inches = SLOT_HEIGHTS[i];
        slot_positions[i].servo_angle = SLOT_ANGLES[i];
        slot_positions[i].is_configured = true;
    }
}

//* ************************************************************************
//* ************************ SLOT MANAGEMENT FUNCTIONS *******************
//* ************************************************************************
void setSlotPosition(int slot_number, float height_inches, int servo_angle) {
    // Validate slot number
    if (slot_number >= 0 && slot_number < TOTAL_SLOTS) {
        slot_positions[slot_number].height_inches = height_inches;
        slot_positions[slot_number].servo_angle = servo_angle;
        slot_positions[slot_number].is_configured = true;
    }
}

SlotPosition getSlotPosition(int slot_number) {
    // Return slot position or default if invalid
    if (slot_number >= 0 && slot_number < TOTAL_SLOTS) {
        return slot_positions[slot_number];
    } else {
        SlotPosition default_slot = {DEFAULT_HEIGHT_INCHES, DEFAULT_SERVO_ANGLE, false};
        return default_slot;
    }
}

int getSlotHeightSteps(int slot_number) {
    // Convert slot height from inches to steps
    if (slot_number >= 0 && slot_number < TOTAL_SLOTS) {
        return (int)(slot_positions[slot_number].height_inches * STEPS_PER_INCH);
    } else {
        return (int)(DEFAULT_HEIGHT_INCHES * STEPS_PER_INCH);
    }
}

void resetSlotConfig() {
    // Reset all slots to default configuration
    initializeSlotConfig();
}

void printSlotConfig() {
    // Print current slot configuration (for debugging)
    for (int i = 0; i < TOTAL_SLOTS; i++) {
        Serial.print("Slot ");
        Serial.print(i);
        Serial.print(": Height=");
        Serial.print(slot_positions[i].height_inches);
        Serial.print(" inches, Angle=");
        Serial.print(slot_positions[i].servo_angle);
        Serial.println(" degrees");
    }
} 