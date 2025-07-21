# Cell Configuration System

## Overview
The cell configuration system allows you to set individual height and servo angle positions for each of the 20 storage cells in your paint machine. This provides precise control over where the loader moves for each cell.

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
static const float CELL_HEIGHTS[TOTAL_CELLS] = {
    3.0,  // Cell 0
    3.2,  // Cell 1
    3.4,  // Cell 2
    // ... continue for all 20 cells
};

static const int CELL_ANGLES[TOTAL_CELLS] = {
    45,   // Cell 0
    47,   // Cell 1
    49,   // Cell 2
    // ... continue for all 20 cells
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
Shows height and angle for all 20 cells.

### Set Individual Cell Position
```
cell_set <cell_number> <height> <angle>
```
Examples:
- `cell_set 5 4.2 55` - Set cell 5 to 4.2 inches height, 55 degrees angle
- `cell_set 0 3.0 45` - Set cell 0 to 3.0 inches height, 45 degrees angle

### Move to Specific Cell
```
cell_move <cell_number>
```
Examples:
- `cell_move 5` - Move Z-axis and servo to cell 5 position
- `cell_move 0` - Move Z-axis and servo to cell 0 position

## Using Cells in Code

### Get Cell Information
```cpp
// Get cell position data
CellPosition pos = getCellPosition(cellNumber);
float height = pos.height_inches;
int angle = pos.servo_angle;

// Get height in steps (for motor control)
int heightSteps = getCellHeightSteps(cellNumber);
```

### Set Cell Position Programmatically
```cpp
// Set cell 5 to new position
setCellPosition(5, 4.2, 55);
```

### Example in State Machine
```cpp
// In a state machine, replace fixed positions with cell positions:
int cellNumber = 5; // Use cell 5
CellPosition cellPos = getCellPosition(cellNumber);
int targetHeightSteps = getCellHeightSteps(cellNumber);

// Move Z motor to cell height
zMotor->moveTo(targetHeightSteps);

// Move servo to cell angle
servoController->moveTo(cellPos.servo_angle);
```

## Cell Numbering
- Cells are numbered 0-19 (20 total cells)
- Cell 0 is the first cell
- Cell 19 is the last cell

## Default Values
- Default height: 5.0 inches
- Default servo angle: 90 degrees
- These are used if an invalid cell number is provided

## Functions Available
- `initializeCellConfig()` - Initialize all cells with default values
- `setCellPosition(cell, height, angle)` - Set individual cell position
- `getCellPosition(cell)` - Get cell position data
- `getCellHeightSteps(cell)` - Get cell height in motor steps
- `resetCellConfig()` - Reset all cells to default configuration
- `printCellConfig()` - Print all cell configurations to serial

## Tips
1. **Test Positions**: Use `cell_move` command to test positions before finalizing
2. **Incremental Changes**: Make small adjustments to height/angle values
3. **Documentation**: Keep notes of which cells work best for different operations
4. **Backup**: Save your working cell configurations in a separate file 