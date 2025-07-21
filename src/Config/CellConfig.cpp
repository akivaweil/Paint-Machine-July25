#include <Arduino.h>
#include "Config/CellConfig.h"
#include "Config/Config.h"

//* ************************************************************************
//* ************************ CELL CONFIGURATION ***************************
//* ************************************************************************
// Storage cell configuration for 20 cells (4 columns x 5 rows) with height and servo angle settings

//* ************************************************************************
//* ************************ GLOBAL VARIABLES *****************************
//* ************************************************************************
static CellPosition cell_positions[TOTAL_CELLS];

//* ************************************************************************
//* ************************ CELL POSITION SETTINGS **********************
//* ************************************************************************
// Easy-to-program cell configuration - modify these values as needed
// Format: {height_inches, servo_angle}
// Columns: A, B, C, D (4 columns)
// Rows: 1, 2, 3, 4, 5 (5 rows)
// Cell A1 is top-left, D5 is bottom-right

//! ************************************************************************
//! CELL CONFIGURATION - MODIFY THESE VALUES FOR EACH CELL
//! ************************************************************************
// Height values (in inches) - easy to program pattern
static const float CELL_HEIGHTS[TOTAL_CELLS] = {
    // Column A (A1, A2, A3, A4, A5)
    3.0, 3.2, 3.4, 3.6, 3.8,
    // Column B (B1, B2, B3, B4, B5)
    4.0, 4.2, 4.4, 4.6, 4.8,
    // Column C (C1, C2, C3, C4, C5)
    5.0, 5.2, 5.4, 5.6, 5.8,
    // Column D (D1, D2, D3, D4, D5)
    6.0, 6.2, 6.4, 6.6, 6.8
};

// Angle values (in degrees) - easy to program pattern
static const int CELL_ANGLES[TOTAL_CELLS] = {
    // Column A (A1, A2, A3, A4, A5)
    45, 47, 49, 51, 53,
    // Column B (B1, B2, B3, B4, B5)
    55, 57, 59, 61, 63,
    // Column C (C1, C2, C3, C4, C5)
    65, 67, 69, 71, 73,
    // Column D (D1, D2, D3, D4, D5)
    75, 77, 79, 81, 83
};

//* ************************************************************************
//* ************************ UTILITY FUNCTIONS ***************************
//* ************************************************************************
int cellToIndex(char column, int row) {
    // Convert column/row to array index
    // Column A=0, B=1, C=2, D=3
    // Row 1=0, 2=1, 3=2, 4=3, 5=4
    int col_index = column - 'A';  // A=0, B=1, C=2, D=3
    int row_index = row - 1;       // 1=0, 2=1, 3=2, 4=3, 5=4
    
    if (col_index >= 0 && col_index < TOTAL_COLUMNS && 
        row_index >= 0 && row_index < TOTAL_ROWS) {
        return col_index * TOTAL_ROWS + row_index;
    }
    return -1; // Invalid cell
}

void indexToCell(int cell_index, char& column, int& row) {
    // Convert array index to column/row
    if (cell_index >= 0 && cell_index < TOTAL_CELLS) {
        int col_index = cell_index / TOTAL_ROWS;
        int row_index = cell_index % TOTAL_ROWS;
        column = 'A' + col_index;
        row = row_index + 1;
    } else {
        column = 'X'; // Invalid
        row = 0;
    }
}

bool isValidCell(char column, int row) {
    return cellToIndex(column, row) >= 0;
}

bool isValidCellIndex(int cell_index) {
    return cell_index >= 0 && cell_index < TOTAL_CELLS;
}

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
void setCellPosition(char column, int row, float height_inches, int servo_angle) {
    // Set cell position using column/row format
    int cell_index = cellToIndex(column, row);
    if (cell_index >= 0) {
        cell_positions[cell_index].height_inches = height_inches;
        cell_positions[cell_index].servo_angle = servo_angle;
        cell_positions[cell_index].is_configured = true;
    }
}

void setCellPositionByIndex(int cell_index, float height_inches, int servo_angle) {
    // Set cell position using array index
    if (isValidCellIndex(cell_index)) {
        cell_positions[cell_index].height_inches = height_inches;
        cell_positions[cell_index].servo_angle = servo_angle;
        cell_positions[cell_index].is_configured = true;
    }
}

CellPosition getCellPosition(char column, int row) {
    // Get cell position using column/row format
    int cell_index = cellToIndex(column, row);
    if (cell_index >= 0) {
        return cell_positions[cell_index];
    } else {
        CellPosition default_cell = {DEFAULT_HEIGHT_INCHES, DEFAULT_SERVO_ANGLE, false};
        return default_cell;
    }
}

CellPosition getCellPositionByIndex(int cell_index) {
    // Get cell position using array index
    if (isValidCellIndex(cell_index)) {
        return cell_positions[cell_index];
    } else {
        CellPosition default_cell = {DEFAULT_HEIGHT_INCHES, DEFAULT_SERVO_ANGLE, false};
        return default_cell;
    }
}

int getCellHeightSteps(char column, int row) {
    // Convert cell height from inches to steps using column/row format
    int cell_index = cellToIndex(column, row);
    if (cell_index >= 0) {
        return (int)(cell_positions[cell_index].height_inches * STEPS_PER_INCH);
    } else {
        return (int)(DEFAULT_HEIGHT_INCHES * STEPS_PER_INCH);
    }
}

int getCellHeightStepsByIndex(int cell_index) {
    // Convert cell height from inches to steps using array index
    if (isValidCellIndex(cell_index)) {
        return (int)(cell_positions[cell_index].height_inches * STEPS_PER_INCH);
    } else {
        return (int)(DEFAULT_HEIGHT_INCHES * STEPS_PER_INCH);
    }
}

void resetCellConfig() {
    // Reset all cells to default configuration
    initializeCellConfig();
}

void printCellConfig() {
    // Print current cell configuration in a grid format
    Serial.println("=== CELL CONFIGURATION GRID ===");
    Serial.println("Format: Cell (Height inches, Angle degrees)");
    Serial.println();
    
    // Print header
    Serial.print("     ");
    for (char col = 'A'; col < 'A' + TOTAL_COLUMNS; col++) {
        Serial.print("Col " + String(col) + "    ");
    }
    Serial.println();
    
    // Print each row
    for (int row = 1; row <= TOTAL_ROWS; row++) {
        Serial.print("Row " + String(row) + " ");
        for (char col = 'A'; col < 'A' + TOTAL_COLUMNS; col++) {
            CellPosition pos = getCellPosition(col, row);
            Serial.print("(" + String(pos.height_inches, 1) + "," + String(pos.servo_angle) + ") ");
        }
        Serial.println();
    }
    Serial.println();
} 