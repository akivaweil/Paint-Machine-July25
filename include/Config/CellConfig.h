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
#define CELLS_PER_COLUMN_A 5  // Column A has 5 cells
#define CELLS_PER_COLUMN_B 4  // Column B has 4 cells (removed highest)
#define CELLS_PER_COLUMN_C 4  // Column C has 4 cells (removed highest)
#define CELLS_PER_COLUMN_D 3  // Column D has 3 cells (removed highest 2)
#define TOTAL_CELLS (CELLS_PER_COLUMN_A + CELLS_PER_COLUMN_B + CELLS_PER_COLUMN_C + CELLS_PER_COLUMN_D)  // 16 cells total
#define DEFAULT_HEIGHT_INCHES 5.0
#define DEFAULT_SERVO_ANGLE 90

//* ************************************************************************
//* ************************ LOADING TRAY CONFIGURATION ******************
//* ************************************************************************
// Loading tray position settings (defined in Config.cpp)

//* ************************************************************************
//* ************************ UTILITY FUNCTIONS ***************************
//* ************************************************************************
int cellToIndex(char column, int row);
void indexToCell(int cell_index, char& column, int& row);
bool isValidCell(char column, int row);
bool isValidCellIndex(int cell_index);

#endif // CELL_CONFIG_H 