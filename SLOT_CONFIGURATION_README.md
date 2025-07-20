# Slot Configuration System

## Overview
The slot configuration system allows you to set individual height and servo angle positions for each of the 20 storage slots in your paint machine. This provides precise control over where the loader moves for each slot.

## Files
- `include/Config/SlotConfig.h` - Header file with structures and function declarations
- `src/Config/SlotConfig.cpp` - Implementation file with slot position settings

## Quick Configuration

### 1. Modify Slot Positions
Edit the arrays in `src/Config/SlotConfig.cpp`:

```cpp
//! ************************************************************************
//! SLOT CONFIGURATION - MODIFY THESE VALUES FOR EACH SLOT
//! ************************************************************************
static const float SLOT_HEIGHTS[TOTAL_SLOTS] = {
    3.0,  // Slot 0
    3.2,  // Slot 1
    3.4,  // Slot 2
    // ... continue for all 20 slots
};

static const int SLOT_ANGLES[TOTAL_SLOTS] = {
    45,   // Slot 0
    47,   // Slot 1
    49,   // Slot 2
    // ... continue for all 20 slots
};
```

### 2. Upload Changes
After modifying the values, upload the code:
```bash
pio run -t upload
```

## Serial Commands

### View All Slot Configurations
```
slots
```
Shows height and angle for all 20 slots.

### Set Individual Slot Position
```
slot_set <slot_number> <height> <angle>
```
Examples:
- `slot_set 5 4.2 55` - Set slot 5 to 4.2 inches height, 55 degrees angle
- `slot_set 0 3.0 45` - Set slot 0 to 3.0 inches height, 45 degrees angle

### Move to Specific Slot
```
slot_move <slot_number>
```
Examples:
- `slot_move 5` - Move Z-axis and servo to slot 5 position
- `slot_move 0` - Move Z-axis and servo to slot 0 position

## Using Slots in Code

### Get Slot Information
```cpp
// Get slot position data
SlotPosition pos = getSlotPosition(slotNumber);
float height = pos.height_inches;
int angle = pos.servo_angle;

// Get height in steps (for motor control)
int heightSteps = getSlotHeightSteps(slotNumber);
```

### Set Slot Position Programmatically
```cpp
// Set slot 5 to new position
setSlotPosition(5, 4.2, 55);
```

### Example in State Machine
```cpp
// In a state machine, replace fixed positions with slot positions:
int slotNumber = 5; // Use slot 5
SlotPosition slotPos = getSlotPosition(slotNumber);
int targetHeightSteps = getSlotHeightSteps(slotNumber);

// Move Z motor to slot height
zMotor->moveTo(targetHeightSteps);

// Move servo to slot angle
servoController->moveTo(slotPos.servo_angle);
```

## Slot Numbering
- Slots are numbered 0-19 (20 total slots)
- Slot 0 is the first slot
- Slot 19 is the last slot

## Default Values
- Default height: 5.0 inches
- Default servo angle: 90 degrees
- These are used if an invalid slot number is provided

## Functions Available
- `initializeSlotConfig()` - Initialize all slots with default values
- `setSlotPosition(slot, height, angle)` - Set individual slot position
- `getSlotPosition(slot)` - Get slot position data
- `getSlotHeightSteps(slot)` - Get slot height in motor steps
- `resetSlotConfig()` - Reset all slots to default configuration
- `printSlotConfig()` - Print all slot configurations to serial

## Tips
1. **Test Positions**: Use `slot_move` command to test positions before finalizing
2. **Incremental Changes**: Make small adjustments to height/angle values
3. **Documentation**: Keep notes of which slots work best for different operations
4. **Backup**: Save your working slot configurations in a separate file 