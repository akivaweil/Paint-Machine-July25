#include "ServoAccelerationController.h"
#include "ServoControl.h"
#include "config/Config.h"
#include "config/Pins_Definitions.h"

//* ************************************************************************
//* ************************ SERVO ACCELERATION EXAMPLE ***************************
//* ************************************************************************
//* This file demonstrates how to use the ServoAccelerationController
//* with the existing ServoControl class
//* 
//* NEW SIMPLIFIED INTERFACE:
//* moveToWithCurve(targetAngle, accelerationCurve)
//* - targetAngle: The desired servo position (0-180 degrees)
//* - accelerationCurve: 0-100 value controlling motion characteristics
//*   - 0-20: Very slow and smooth (gentle motion)
//*   - 20-50: Moderate speed and acceleration
//*   - 50-80: Fast motion with quick acceleration
//*   - 80-100: Very fast and aggressive motion
//* ************************************************************************

// Create servo and acceleration controller instances
ServoControl myServo;
ServoAccelerationController servoController(&myServo);

// Example usage functions
void setupServoWithAcceleration() {
    // Initialize the servo
    myServo.init(LOADER_SERVO_PIN, 7); // Using channel 7 for servo
    
    // Configure acceleration profile
    servoController.setAccelerationProfile(
        0.002,  // Acceleration rate (degrees/ms^2) - same as deceleration
        0.1     // Max velocity (degrees/ms)
    );
    
    // Set initial position
    servoController.setCurrentAngle(90.0);
    myServo.write(90.0);
}

void moveServoSmoothly() {
    // Move to 0 degrees with very slow, smooth acceleration (curve = 10)
    servoController.moveToWithCurve(0.0, 10);
    
    // Wait for movement to complete
    while (servoController.isMoving()) {
        servoController.update();
        delay(10); // Update every 10ms for smooth motion
    }
    
    // Move to 180 degrees with medium acceleration (curve = 50)
    servoController.moveToWithCurve(180.0, 50);
    
    // Wait for movement to complete
    while (servoController.isMoving()) {
        servoController.update();
        delay(10);
    }
    
    // Move back to center with fast, aggressive acceleration (curve = 90)
    servoController.moveToWithCurve(90.0, 90);
    
    while (servoController.isMoving()) {
        servoController.update();
        delay(10);
    }
}

void emergencyStop() {
    // Stop the servo immediately with deceleration
    servoController.stop();
    
    // Continue updating until stopped
    while (servoController.isMoving()) {
        servoController.update();
        delay(10);
    }
}

// Example of how to integrate with main loop
void updateServoInMainLoop() {
    // Call this in your main loop
    servoController.update();
    
    // Check if servo has reached target
    if (servoController.hasReachedTarget()) {
        // Servo has completed its movement
        // You can start the next movement here
    }
    
    // Get current status
    float currentAngle = servoController.getCurrentAngle();
    float currentVelocity = servoController.getCurrentVelocity();
    ServoAccelerationController::MotionState state = servoController.getMotionState();
}

// Example demonstrating different acceleration curves
void demonstrateAccelerationCurves() {
    // Very gentle motion (curve = 5)
    servoController.moveToWithCurve(45.0, 5);
    while (servoController.isMoving()) {
        servoController.update();
        delay(10);
    }
    
    // Moderate motion (curve = 30)
    servoController.moveToWithCurve(135.0, 30);
    while (servoController.isMoving()) {
        servoController.update();
        delay(10);
    }
    
    // Fast motion (curve = 70)
    servoController.moveToWithCurve(90.0, 70);
    while (servoController.isMoving()) {
        servoController.update();
        delay(10);
    }
    
    // Very aggressive motion (curve = 95)
    servoController.moveToWithCurve(0.0, 95);
    while (servoController.isMoving()) {
        servoController.update();
        delay(10);
    }
} 