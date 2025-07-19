#ifndef CYLINDER_CONTROL_H
#define CYLINDER_CONTROL_H

#include "Config/Pins_Definitions.h"

//* ************************************************************************
//* ************************ CYLINDER CONTROL ******************************
//* ************************************************************************
// Extension cylinder control class for pneumatic cylinder operations
// Controls relay-based cylinder extension and retraction

class CylinderControl {
private:
    int cylinderPin;           // Pin connected to cylinder relay
    bool isExtended;          // Current state of the cylinder
    
public:
    // Constructor
    CylinderControl(int pin = EXTENSION_CYLINDER_PIN);
    
    // Initialization
    void begin();
    
    // Cylinder control functions
    void extend();            // Extend the cylinder (HIGH)
    void retract();           // Retract the cylinder (LOW)
    void toggle();            // Toggle between extended/retracted states
    
    // Status functions
    bool getState();          // Get current cylinder state (true = extended, false = retracted)
    bool isCylinderExtended(); // Check if cylinder is currently extended
};

#endif 