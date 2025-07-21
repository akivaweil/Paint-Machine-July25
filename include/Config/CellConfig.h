#ifndef CELL_CONFIG_H
#define CELL_CONFIG_H

//* ************************************************************************
//* ************************ CELL CONFIGURATION ***************************
//* ************************************************************************
// Storage cell configuration for 20 cells (4 columns x 5 rows) with height and servo angle settings

//* ************************************************************************
//* ************************ CELL STRUCTURE *******************************
//* ************************************************************************
struct CellPosition {
    float height_inches;    // Height position in inches
    int servo_angle;        // Servo angle in degrees
    bool is_configured;     // Whether this cell has been configured
};

//* ************************************************************************
//* ************************ FUNCTION DECLARATIONS ***********************
//* ************************************************************************
void initializeCellConfig();
void setCellPosition(char column, int row, float height_inches, int servo_angle);
void setCellPositionByIndex(int cell_index, float height_inches, int servo_angle);
CellPosition getCellPosition(char column, int row);
CellPosition getCellPositionByIndex(int cell_index);
int getCellHeightSteps(char column, int row);
int getCellHeightStepsByIndex(int cell_index);
void resetCellConfig();
void printCellConfig();

//* ************************************************************************
//* ************************ CONSTANTS ***********************************
//* ************************************************************************
#define TOTAL_COLUMNS 4
#define TOTAL_ROWS 5
#define TOTAL_CELLS (TOTAL_COLUMNS * TOTAL_ROWS)  // 20 cells total
#define DEFAULT_HEIGHT_INCHES 5.0
#define DEFAULT_SERVO_ANGLE 90

//* ************************************************************************
//* ************************ UTILITY FUNCTIONS ***************************
//* ************************************************************************
int cellToIndex(char column, int row);
void indexToCell(int cell_index, char& column, int& row);
bool isValidCell(char column, int row);
bool isValidCellIndex(int cell_index);

#endif // CELL_CONFIG_H 