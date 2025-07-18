#include "ServoAccelerationController.h"
#include "ServoControl.h"
#include "config/Config.h"
#include "config/Pins_Definitions.h"

//* ************************************************************************
//* ************************ SERVO ACCELERATION EXAMPLE ***************************
//* ************************************************************************
//* This file demonstrates how to use the ServoAccelerationController
//* with the existing ServoControl class
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
        0.002,  // Acceleration rate (degrees/ms^2)
        0.002,  // Deceleration rate (degrees/ms^2)
        0.1     // Max velocity (degrees/ms)
    );
    
    // Set initial position
    servoController.setCurrentAngle(90.0);
    myServo.write(90.0);
}

void moveServoSmoothly() {
    // Move to 0 degrees with smooth acceleration
    servoController.moveTo(0.0);
    
    // Wait for movement to complete
    while (servoController.isMoving()) {
        servoController.update();
        delay(10); // Update every 10ms for smooth motion
    }
    
    // Move to 180 degrees in exactly 2 seconds
    servoController.moveToWithTime(180.0, 2000);
    
    // Wait for movement to complete
    while (servoController.isMoving()) {
        servoController.update();
        delay(10);
    }
    
    // Move back to center with custom acceleration
    servoController.setAccelerationRate(0.005); // Faster acceleration
    servoController.setDecelerationRate(0.001); // Slower deceleration
    servoController.moveTo(90.0);
    
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