#include <Arduino.h>
#include "Config/CellConfig.h"
#include "Config/Config.h"

//* ************************************************************************
//* ************************ EASY CELL CONFIGURATION GRID *****************
//* ************************************************************************
// Edit the values below to set the height (inches) and angle (degrees) for each cell.
// Columns: A, B, C, D (left to right)
// Rows:    1, 2, 3, 4, 5 (top to bottom)
// Example: CELL_HEIGHTS[0][0] is A1, CELL_HEIGHTS[3][4] is D5
//          CELL_ANGLES[1][2] is B3
//
//        A      B      C      D
// 1   {3.0,  4.0,  5.0,  6.0},
// 2   {3.2,  4.2,  5.2,  6.2},
// 3   {3.4,  4.4,  5.4,  6.4},
// 4   {3.6,  4.6,  5.6,  6.6},
// 5   {3.8,  4.8,  5.8,  6.8}

float CELL_HEIGHTS[TOTAL_ROWS][TOTAL_COLUMNS] = {
    {3.0,  4.0,  5.0,  6.0}, // Row 1 (A1, B1, C1, D1)
    {3.2,  4.2,  5.2,  6.2}, // Row 2 (A2, B2, C2, D2)
    {3.4,  4.4,  5.4,  6.4}, // Row 3 (A3, B3, C3, D3)
    {3.6,  4.6,  5.6,  6.6}, // Row 4 (A4, B4, C4, D4)
    {3.8,  4.8,  5.8,  6.8}  // Row 5 (A5, B5, C5, D5)
};

int CELL_ANGLES[TOTAL_ROWS][TOTAL_COLUMNS] = {
    {152,  55,  65,  75}, // Row 1
    {152,  57,  67,  77}, // Row 2
    {152,  59,  69,  79}, // Row 3
    {152,  61,  71,  81}, // Row 4
    {152,  63,  73,  83}  // Row 5
};

//* ************************************************************************
//* ************************ INTERNAL FLAT ARRAYS *************************
//* ************************************************************************
static CellPosition cell_positions[TOTAL_CELLS];

//* ************************************************************************
//* ************************ UTILITY FUNCTIONS ***************************
//* ************************************************************************
int cellToIndex(char column, int row) {
    int col_index = column - 'A';
    int row_index = row - 1;
    if (col_index >= 0 && col_index < TOTAL_COLUMNS && row_index >= 0 && row_index < TOTAL_ROWS) {
        return row_index * TOTAL_COLUMNS + col_index;
    }
    return -1;
}

void indexToCell(int cell_index, char& column, int& row) {
    if (cell_index >= 0 && cell_index < TOTAL_CELLS) {
        int row_index = cell_index / TOTAL_COLUMNS;
        int col_index = cell_index % TOTAL_COLUMNS;
        column = 'A' + col_index;
        row = row_index + 1;
    } else {
        column = 'X';
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
    // Flatten the 2D grid into the internal array
    for (int row = 0; row < TOTAL_ROWS; row++) {
        for (int col = 0; col < TOTAL_COLUMNS; col++) {
            int idx = row * TOTAL_COLUMNS + col;
            cell_positions[idx].height_inches = CELL_HEIGHTS[row][col];
            cell_positions[idx].servo_angle = CELL_ANGLES[row][col];
            cell_positions[idx].is_configured = true;
        }
    }
}

//* ************************************************************************
//* ************************ CELL MANAGEMENT FUNCTIONS *******************
//* ************************************************************************
void setCellPosition(char column, int row, float height_inches, int servo_angle) {
    int col_index = column - 'A';
    int row_index = row - 1;
    int cell_index = cellToIndex(column, row);
    if (cell_index >= 0) {
        CELL_HEIGHTS[row_index][col_index] = height_inches;
        CELL_ANGLES[row_index][col_index] = servo_angle;
        cell_positions[cell_index].height_inches = height_inches;
        cell_positions[cell_index].servo_angle = servo_angle;
        cell_positions[cell_index].is_configured = true;
    }
}

void setCellPositionByIndex(int cell_index, float height_inches, int servo_angle) {
    if (isValidCellIndex(cell_index)) {
        char column; int row;
        indexToCell(cell_index, column, row);
        setCellPosition(column, row, height_inches, servo_angle);
    }
}

CellPosition getCellPosition(char column, int row) {
    int cell_index = cellToIndex(column, row);
    if (cell_index >= 0) {
        return cell_positions[cell_index];
    } else {
        CellPosition default_cell = {DEFAULT_HEIGHT_INCHES, DEFAULT_SERVO_ANGLE, false};
        return default_cell;
    }
}

CellPosition getCellPositionByIndex(int cell_index) {
    if (isValidCellIndex(cell_index)) {
        return cell_positions[cell_index];
    } else {
        CellPosition default_cell = {DEFAULT_HEIGHT_INCHES, DEFAULT_SERVO_ANGLE, false};
        return default_cell;
    }
}

int getCellHeightSteps(char column, int row) {
    int cell_index = cellToIndex(column, row);
    if (cell_index >= 0) {
        return (int)(cell_positions[cell_index].height_inches * STEPS_PER_INCH);
    } else {
        return (int)(DEFAULT_HEIGHT_INCHES * STEPS_PER_INCH);
    }
}

int getCellHeightStepsByIndex(int cell_index) {
    if (isValidCellIndex(cell_index)) {
        return (int)(cell_positions[cell_index].height_inches * STEPS_PER_INCH);
    } else {
        return (int)(DEFAULT_HEIGHT_INCHES * STEPS_PER_INCH);
    }
}

void resetCellConfig() {
    initializeCellConfig();
}

void printCellConfig() {
    Serial.println("=== CELL CONFIGURATION GRID ===");
    Serial.println("Format: Cell (Height inches, Angle degrees)");
    Serial.println();
    Serial.print("     ");
    for (char col = 'A'; col < 'A' + TOTAL_COLUMNS; col++) {
        Serial.print("Col " + String(col) + "    ");
    }
    Serial.println();
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