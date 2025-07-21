#include <Arduino.h>
#include "Config/CellConfig.h"
#include "Config/Config.h"

//* ************************************************************************
//* ************************ CELL CONFIGURATION ***************************
//* ************************************************************************
// Storage cell configuration for 20 cells with height and servo angle settings

//* ************************************************************************
//* ************************ GLOBAL VARIABLES *****************************
//* ************************************************************************
static CellPosition cell_positions[TOTAL_CELLS];

//* ************************************************************************
//* ************************ CELL POSITION SETTINGS **********************
//* ************************************************************************
// Easy-to-read cell configuration - modify these values as needed
// Format: {height_inches, servo_angle}
// Cell numbers are 0-19 (20 total cells)

//! ************************************************************************
//! CELL CONFIGURATION - MODIFY THESE VALUES FOR EACH CELL
//! ************************************************************************
static const float CELL_HEIGHTS[TOTAL_CELLS] = {
    3.0,  // Cell 0
    3.2,  // Cell 1
    3.4,  // Cell 2
    3.6,  // Cell 3
    3.8,  // Cell 4
    4.0,  // Cell 5
    4.2,  // Cell 6
    4.4,  // Cell 7
    4.6,  // Cell 8
    4.8,  // Cell 9
    5.0,  // Cell 10
    5.2,  // Cell 11
    5.4,  // Cell 12
    5.6,  // Cell 13
    5.8,  // Cell 14
    6.0,  // Cell 15
    6.2,  // Cell 16
    6.4,  // Cell 17
    6.6,  // Cell 18
    6.8   // Cell 19
};

static const int CELL_ANGLES[TOTAL_CELLS] = {
    45,   // Cell 0
    47,   // Cell 1
    49,   // Cell 2
    51,   // Cell 3
    53,   // Cell 4
    55,   // Cell 5
    57,   // Cell 6
    59,   // Cell 7
    61,   // Cell 8
    63,   // Cell 9
    65,   // Cell 10
    67,   // Cell 11
    69,   // Cell 12
    71,   // Cell 13
    73,   // Cell 14
    75,   // Cell 15
    77,   // Cell 16
    79,   // Cell 17
    81,   // Cell 18
    83    // Cell 19
};

//* ************************************************************************
//* ************************ INITIALIZATION *******************************
//* ************************************************************************
void initializeCellConfig() {
    // Initialize all cells with default values
    for (int i = 0; i < TOTAL_CELLS; i++) {
        cell_positions[i].height_inches = CELL_HEIGHTS[i];
        cell_positions[i].servo_angle = CELL_ANGLES[i];
        cell_positions[i].is_configured = true;
    }
}

//* ************************************************************************
//* ************************ CELL MANAGEMENT FUNCTIONS *******************
//* ************************************************************************
void setCellPosition(int cell_number, float height_inches, int servo_angle) {
    // Validate cell number
    if (cell_number >= 0 && cell_number < TOTAL_CELLS) {
        cell_positions[cell_number].height_inches = height_inches;
        cell_positions[cell_number].servo_angle = servo_angle;
        cell_positions[cell_number].is_configured = true;
    }
}

CellPosition getCellPosition(int cell_number) {
    // Return cell position or default if invalid
    if (cell_number >= 0 && cell_number < TOTAL_CELLS) {
        return cell_positions[cell_number];
    } else {
        CellPosition default_cell = {DEFAULT_HEIGHT_INCHES, DEFAULT_SERVO_ANGLE, false};
        return default_cell;
    }
}

int getCellHeightSteps(int cell_number) {
    // Convert cell height from inches to steps
    if (cell_number >= 0 && cell_number < TOTAL_CELLS) {
        return (int)(cell_positions[cell_number].height_inches * STEPS_PER_INCH);
    } else {
        return (int)(DEFAULT_HEIGHT_INCHES * STEPS_PER_INCH);
    }
}

void resetCellConfig() {
    // Reset all cells to default configuration
    initializeCellConfig();
}

void printCellConfig() {
    // Print current cell configuration (for debugging)
    for (int i = 0; i < TOTAL_CELLS; i++) {
        Serial.print("Cell ");
        Serial.print(i);
        Serial.print(": Height=");
        Serial.print(cell_positions[i].height_inches);
        Serial.print(" inches, Angle=");
        Serial.print(cell_positions[i].servo_angle);
        Serial.println(" degrees");
    }
} 