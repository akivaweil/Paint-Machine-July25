#include <Arduino.h>
#include "Config/CellConfig.h"
#include "Config/Config.h"

//* ************************************************************************
//* ************************ LOADING TRAY CONFIGURATION ******************
//* ************************************************************************
// Loading tray position and fork extension settings
// These values are used by the cell sequence state for loading tray operations

//* ************************************************************************
//* ************************ EASY CELL CONFIGURATION GRID *****************
//* ************************************************************************
// Edit the values below to set the height (inches) and angle (degrees) for each cell.
// Columns: A, B, C, D (left to right)
// Rows:    1, 2, 3, 4, 5 (top to bottom)
// Note: Column B has 4 cells (removed highest), Column D has 3 cells (removed highest 2)
// Example: CELL_HEIGHTS[0][0] is A1, CELL_HEIGHTS[3][4] is D5
//          CELL_ANGLES[1][2] is B3
//
//        A      B      C      D
// 1   {3.0,  4.0,  5.0,  6.0},  // B1 and D1 removed
// 2   {3.2,  4.2,  5.2,  6.2},  // D2 removed
// 3   {3.4,  4.4,  5.4,  6.4},
// 4   {3.6,  4.6,  5.6,  6.6},
// 5   {3.8,  4.8,  5.8,  6.8}

float CELL_HEIGHTS[TOTAL_ROWS][TOTAL_COLUMNS] = {
    {8.3  0.0,  9.645,  0.0}, // Row 1 (A1, B1 removed, C1, D1 removed)
    {6.3,  7.487,  7.716,  0.0}, // Row 2 (A2, B2, C2, D2 removed)
    {4.3,  5.56,  5.787,  5.56}, // Row 3 (A3, B3, C3, D3)
    {2.3,  3.629,  3.858,  3.629}, // Row 4 (A4, B4, C4, D4)
    {0.3,  1.7,  2.665,  1.7}  // Row 5 (A5, B5, C5, D5)
};

int CELL_ANGLES[TOTAL_ROWS][TOTAL_COLUMNS] = {
    {173,  0,  126,  0}, // Row 1 (B1 and D1 removed)
    {173,  148,  126,  0}, // Row 2 (D2 removed)
    {173,  148,  126,  104}, // Row 3
    {173,  148,  126,  104}, // Row 4
    {173,  148,  126,  104}  // Row 5
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
    
    // Check if the cell exists based on column configuration
    if (col_index == 0) { // Column A - 5 cells (rows 1-5)
        if (row_index >= 0 && row_index < CELLS_PER_COLUMN_A) {
            return row_index;
        }
    } else if (col_index == 1) { // Column B - 4 cells (rows 2-5, removed row 1)
        if (row_index >= 1 && row_index < 5) {
            return CELLS_PER_COLUMN_A + (row_index - 1);
        }
    } else if (col_index == 2) { // Column C - 5 cells (rows 1-5)
        if (row_index >= 0 && row_index < CELLS_PER_COLUMN_C) {
            return CELLS_PER_COLUMN_A + CELLS_PER_COLUMN_B + row_index;
        }
    } else if (col_index == 3) { // Column D - 3 cells (rows 3-5, removed rows 1-2)
        if (row_index >= 2 && row_index < 5) {
            return CELLS_PER_COLUMN_A + CELLS_PER_COLUMN_B + CELLS_PER_COLUMN_C + (row_index - 2);
        }
    }
    return -1; // Invalid cell
}

void indexToCell(int cell_index, char& column, int& row) {
    if (cell_index >= 0 && cell_index < TOTAL_CELLS) {
        if (cell_index < CELLS_PER_COLUMN_A) {
            // Column A cells (0-4)
            column = 'A';
            row = cell_index + 1;
        } else if (cell_index < CELLS_PER_COLUMN_A + CELLS_PER_COLUMN_B) {
            // Column B cells (5-8)
            column = 'B';
            row = (cell_index - CELLS_PER_COLUMN_A) + 2; // +2 because row 1 is removed
        } else if (cell_index < CELLS_PER_COLUMN_A + CELLS_PER_COLUMN_B + CELLS_PER_COLUMN_C) {
            // Column C cells (9-13)
            column = 'C';
            row = (cell_index - CELLS_PER_COLUMN_A - CELLS_PER_COLUMN_B) + 1;
        } else {
            // Column D cells (14-16)
            column = 'D';
            row = (cell_index - CELLS_PER_COLUMN_A - CELLS_PER_COLUMN_B - CELLS_PER_COLUMN_C) + 3; // +3 because rows 1-2 are removed
        }
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
    // Initialize all cells as not configured first
    for (int i = 0; i < TOTAL_CELLS; i++) {
        cell_positions[i].is_configured = false;
    }
    
    // Initialize valid cells based on column configuration
    // Column A: 5 cells (rows 1-5)
    for (int row = 0; row < CELLS_PER_COLUMN_A; row++) {
        int idx = row;
        cell_positions[idx].height_inches = CELL_HEIGHTS[row][0];
        cell_positions[idx].servo_angle = CELL_ANGLES[row][0];
        cell_positions[idx].is_configured = true;
    }
    
    // Column B: 4 cells (rows 2-5, removed row 1)
    for (int row = 1; row < 5; row++) {
        int idx = CELLS_PER_COLUMN_A + (row - 1);
        cell_positions[idx].height_inches = CELL_HEIGHTS[row][1];
        cell_positions[idx].servo_angle = CELL_ANGLES[row][1];
        cell_positions[idx].is_configured = true;
    }
    
    // Column C: 5 cells (rows 1-5)
    for (int row = 0; row < CELLS_PER_COLUMN_C; row++) {
        int idx = CELLS_PER_COLUMN_A + CELLS_PER_COLUMN_B + row;
        cell_positions[idx].height_inches = CELL_HEIGHTS[row][2];
        cell_positions[idx].servo_angle = CELL_ANGLES[row][2];
        cell_positions[idx].is_configured = true;
    }
    
    // Column D: 3 cells (rows 3-5, removed rows 1-2)
    for (int row = 2; row < 5; row++) {
        int idx = CELLS_PER_COLUMN_A + CELLS_PER_COLUMN_B + CELLS_PER_COLUMN_C + (row - 2);
        cell_positions[idx].height_inches = CELL_HEIGHTS[row][3];
        cell_positions[idx].servo_angle = CELL_ANGLES[row][3];
        cell_positions[idx].is_configured = true;
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
    Serial.println("Note: Column B has 4 cells (B1 removed), Column D has 3 cells (D1, D2 removed)");
    Serial.println();
    Serial.print("     ");
    for (char col = 'A'; col < 'A' + TOTAL_COLUMNS; col++) {
        Serial.print("Col " + String(col) + "    ");
    }
    Serial.println();
    for (int row = 1; row <= TOTAL_ROWS; row++) {
        Serial.print("Row " + String(row) + " ");
        for (char col = 'A'; col < 'A' + TOTAL_COLUMNS; col++) {
            // Check if this cell exists in the new configuration
            bool cell_exists = false;
            if (col == 'A') {
                cell_exists = (row >= 1 && row <= 5); // All 5 cells exist
            } else if (col == 'B') {
                cell_exists = (row >= 2 && row <= 5); // 4 cells, removed row 1
            } else if (col == 'C') {
                cell_exists = (row >= 1 && row <= 5); // All 5 cells exist
            } else if (col == 'D') {
                cell_exists = (row >= 3 && row <= 5); // 3 cells, removed rows 1-2
            }
            
            if (cell_exists) {
                CellPosition pos = getCellPosition(col, row);
                Serial.print("(" + String(pos.height_inches, 1) + "," + String(pos.servo_angle) + ") ");
            } else {
                Serial.print("   REMOVED   ");
            }
        }
        Serial.println();
    }
    Serial.println();
    Serial.println("Total cells: " + String(TOTAL_CELLS));
    Serial.println("Column A: " + String(CELLS_PER_COLUMN_A) + " cells");
    Serial.println("Column B: " + String(CELLS_PER_COLUMN_B) + " cells");
    Serial.println("Column C: " + String(CELLS_PER_COLUMN_C) + " cells");
    Serial.println("Column D: " + String(CELLS_PER_COLUMN_D) + " cells");
    Serial.println();
} 