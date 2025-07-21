#ifndef CELL_CONFIG_H
#define CELL_CONFIG_H

//* ************************************************************************
//* ************************ CELL CONFIGURATION ***************************
//* ************************************************************************
// Storage cell configuration for 20 cells with height and servo angle settings

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
void setCellPosition(int cell_number, float height_inches, int servo_angle);
CellPosition getCellPosition(int cell_number);
int getCellHeightSteps(int cell_number);
void resetCellConfig();
void printCellConfig();

//* ************************************************************************
//* ************************ CONSTANTS ***********************************
//* ************************************************************************
#define TOTAL_CELLS 20
#define DEFAULT_HEIGHT_INCHES 5.0
#define DEFAULT_SERVO_ANGLE 90

#endif // CELL_CONFIG_H 