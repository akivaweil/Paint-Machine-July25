# Cell Configuration System

## Overview
The cell configuration system allows you to set individual height and servo angle positions for each of the 20 storage cells in your paint machine. The cells are organized in a 4-column by 5-row grid format for easy programming and reference.

## Cell Layout
```
     Col A    Col B    Col C    Col D
Row 1  A1       B1       C1       D1
Row 2  A2       B2       C2       D2  
Row 3  A3       B3       C3       D3
Row 4  A4       B4       C4       D4
Row 5  A5       B5       C5       D5
```

## Files
- `include/Config/CellConfig.h` - Header file with structures and function declarations
- `src/Config/CellConfig.cpp` - Implementation file with cell position settings

## Quick Configuration

### 1. Modify Cell Positions
Edit the arrays in `src/Config/CellConfig.cpp`:

```cpp
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
```

### 2. Upload Changes
After modifying the values, upload the code:
```bash
pio run -t upload
```

## Serial Commands

### View All Cell Configurations
```
cells
```
Shows height and angle for all 20 cells in a grid format.

### Set Individual Cell Position
```
cell_set <column><row> <height> <angle>
```
Examples:
- `cell_set A1 4.2 55` - Set cell A1 to 4.2 inches height, 55 degrees angle
- `cell_set B3 5.0 65` - Set cell B3 to 5.0 inches height, 65 degrees angle
- `cell_set D5 6.8 83` - Set cell D5 to 6.8 inches height, 83 degrees angle

### Move to Specific Cell
```
cell_move <column><row>
```
Examples:
- `cell_move A1` - Move Z-axis and servo to cell A1 position
- `cell_move C3` - Move Z-axis and servo to cell C3 position
- `cell_move D5` - Move Z-axis and servo to cell D5 position

## Using Cells in Code

### Get Cell Information
```cpp
// Get cell position data using column/row format
CellPosition pos = getCellPosition('A', 1);  // Get cell A1
float height = pos.height_inches;
int angle = pos.servo_angle;

// Get height in steps (for motor control)
int heightSteps = getCellHeightSteps('A', 1);  // Get cell A1 height in steps
```

### Set Cell Position Programmatically
```cpp
// Set cell A1 to new position
setCellPosition('A', 1, 4.2, 55);

// Set cell B3 to new position
setCellPosition('B', 3, 5.0, 65);
```

### Example in State Machine
```cpp
// In a state machine, replace fixed positions with cell positions:
char column = 'A'; int row = 1; // Use cell A1
CellPosition cellPos = getCellPosition(column, row);
int targetHeightSteps = getCellHeightSteps(column, row);

// Move Z motor to cell height
zMotor->moveTo(targetHeightSteps);

// Move servo to cell angle
servoController->moveTo(cellPos.servo_angle);
```

## Cell Numbering System
- **Columns**: A, B, C, D (4 columns)
- **Rows**: 1, 2, 3, 4, 5 (5 rows)
- **Cell References**: A1, A2, A3, A4, A5, B1, B2, B3, B4, B5, C1, C2, C3, C4, C5, D1, D2, D3, D4, D5
- **Total Cells**: 20 (4 × 5 grid)

## Default Values
- Default height: 5.0 inches
- Default servo angle: 90 degrees
- These are used if an invalid cell reference is provided

## Functions Available
- `initializeCellConfig()` - Initialize all cells with default values
- `setCellPosition(column, row, height, angle)` - Set individual cell position
- `getCellPosition(column, row)` - Get cell position data
- `getCellHeightSteps(column, row)` - Get cell height in motor steps
- `resetCellConfig()` - Reset all cells to default configuration
- `printCellConfig()` - Print all cell configurations to serial in grid format

## Easy Programming Pattern
The default configuration uses an easy-to-program pattern:

**Heights**: Increase by 0.2 inches across columns, 0.2 inches down rows
- Column A: 3.0, 3.2, 3.4, 3.6, 3.8 inches
- Column B: 4.0, 4.2, 4.4, 4.6, 4.8 inches
- Column C: 5.0, 5.2, 5.4, 5.6, 5.8 inches
- Column D: 6.0, 6.2, 6.4, 6.6, 6.8 inches

**Angles**: Increase by 2 degrees across columns, 2 degrees down rows
- Column A: 45, 47, 49, 51, 53 degrees
- Column B: 55, 57, 59, 61, 63 degrees
- Column C: 65, 67, 69, 71, 73 degrees
- Column D: 75, 77, 79, 81, 83 degrees

## Tips
1. **Test Positions**: Use `cell_move` command to test positions before finalizing
2. **Grid Layout**: Think of cells as a spreadsheet - A1 is top-left, D5 is bottom-right
3. **Easy Programming**: Modify the arrays in CellConfig.cpp to change all cells at once
4. **Documentation**: Keep notes of which cells work best for different operations
5. **Backup**: Save your working cell configurations in a separate file 