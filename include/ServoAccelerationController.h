#ifndef SERVO_ACCELERATION_CONTROLLER_H
#define SERVO_ACCELERATION_CONTROLLER_H

#include <Arduino.h>
#include "ServoControl.h"

class ServoAccelerationController {
public:
    // Motion state enum
    enum MotionState {
        IDLE,
        ACCELERATING,
        CONSTANT_VELOCITY,
        DECELERATING
    };

private:
    ServoControl* servo; // Pointer to the servo to control
    
    // Acceleration curve parameters
    float accelerationRate; // Degrees per second^2
    float decelerationRate; // Degrees per second^2
    float maxVelocity; // Maximum velocity in degrees per second
    
    // Current motion state
    float currentAngle;
    float targetAngle;
    float currentVelocity;
    float startAngle;
    
    // Timing
    unsigned long lastUpdateTime;
    unsigned long moveStartTime;
    
    MotionState currentState;
    
    // Calculate the next position based on acceleration curve
    float calculateNextPosition();
    
    // Update motion state based on current conditions
    void updateMotionState();
    
    // Calculate time to reach target with current parameters
    unsigned long calculateMoveTime(float startPos, float endPos);

public:
    ServoAccelerationController(ServoControl* servoPtr);
    
    // Configuration methods
    void setAccelerationRate(float accelRate);
    void setDecelerationRate(float decelRate);
    void setMaxVelocity(float maxVel);
    void setAccelerationProfile(float accelRate, float maxVel); // Simplified - accel and decel are always the same
    
    // Motion control methods
    void moveTo(float targetAngle);
    void moveToWithTime(float targetAngle, unsigned long moveTimeMs);
    void moveToWithCurve(float targetAngle, int accelerationCurve); // 0-100 curve value
    void stop();
    void update(); // Call this in main loop
    
    // Status methods
    bool isMoving();
    bool hasReachedTarget();
    float getCurrentAngle();
    float getCurrentVelocity();
    MotionState getMotionState();
    
    // Reset methods
    void reset();
    void setCurrentAngle(float angle);
};

#endif 