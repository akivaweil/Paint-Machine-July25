#include "ServoAccelerationController.h"

//* ************************************************************************
//* ************************ SERVO ACCELERATION CONTROLLER ***************************
//* ************************************************************************
//* This class provides smooth acceleration and deceleration curves for servo motion
//* It works with the existing ServoControl class without modifying it
//* ************************************************************************

ServoAccelerationController::ServoAccelerationController(ServoControl* servoPtr) {
    servo = servoPtr;
    
    // Default acceleration profile (conservative values)
    accelerationRate = 0.001; // 1 degree per second^2
    decelerationRate = 0.001; // 1 degree per second^2
    maxVelocity = 0.05; // 50 degrees per second
    
    // Initialize motion state
    currentAngle = 0.0;
    targetAngle = 0.0;
    currentVelocity = 0.0;
    startAngle = 0.0;
    currentState = IDLE;
    
    // Initialize timing
    lastUpdateTime = millis();
    moveStartTime = millis();
}

//* ************************************************************************
//* ************************ CONFIGURATION METHODS ***************************
//* ************************************************************************

void ServoAccelerationController::setAccelerationRate(float accelRate) {
    accelerationRate = accelRate;
}

void ServoAccelerationController::setDecelerationRate(float decelRate) {
    decelerationRate = decelRate;
}

void ServoAccelerationController::setMaxVelocity(float maxVel) {
    maxVelocity = maxVel;
}

void ServoAccelerationController::setAccelerationProfile(float accelRate, float decelRate, float maxVel) {
    accelerationRate = accelRate;
    decelerationRate = decelRate;
    maxVelocity = maxVel;
}

//* ************************************************************************
//* ************************ MOTION CONTROL METHODS ***************************
//* ************************************************************************

void ServoAccelerationController::moveTo(float targetAngle) {
    this->targetAngle = targetAngle;
    startAngle = currentAngle;
    moveStartTime = millis();
    currentState = ACCELERATING;
    currentVelocity = 0.0;
}

void ServoAccelerationController::moveToWithTime(float targetAngle, unsigned long moveTimeMs) {
    // Calculate required acceleration profile to reach target in specified time
    float distance = abs(targetAngle - currentAngle);
    
    if (distance == 0) {
        return; // Already at target
    }
    
    // Calculate required acceleration for triangular velocity profile
    // distance = 0.5 * a * (t/2)^2 * 2 (triangular profile)
    float requiredAccel = (4.0 * distance) / (moveTimeMs * moveTimeMs);
    
    // Set the acceleration profile
    setAccelerationProfile(requiredAccel, requiredAccel, requiredAccel * moveTimeMs / 2.0);
    
    // Start the move
    moveTo(targetAngle);
}

void ServoAccelerationController::stop() {
    currentState = DECELERATING;
    targetAngle = currentAngle; // Stop at current position
}

void ServoAccelerationController::update() {
    if (currentState == IDLE) {
        return;
    }
    
    unsigned long currentTime = millis();
    unsigned long deltaTime = currentTime - lastUpdateTime;
    
    if (deltaTime == 0) {
        return; // No time has passed
    }
    
    // Update motion state
    updateMotionState();
    
    // Calculate new position
    float newAngle = calculateNextPosition();
    
    // Update current angle and velocity
    currentVelocity = (newAngle - currentAngle) / deltaTime;
    currentAngle = newAngle;
    
    // Send command to servo
    servo->write(currentAngle);
    
    // Update timing
    lastUpdateTime = currentTime;
    
    // Check if we've reached the target
    if (abs(currentAngle - targetAngle) < 0.1 && abs(currentVelocity) < 0.01) {
        currentState = IDLE;
        currentVelocity = 0.0;
    }
}

//* ************************************************************************
//* ************************ PRIVATE CALCULATION METHODS ***************************
//* ************************************************************************

void ServoAccelerationController::updateMotionState() {
    float distanceToTarget = abs(targetAngle - currentAngle);
    float distanceFromStart = abs(currentAngle - startAngle);
    
    // Calculate stopping distance with current velocity
    float stoppingDistance = (currentVelocity * currentVelocity) / (2.0 * decelerationRate);
    
    switch (currentState) {
        case ACCELERATING:
            // Check if we should switch to constant velocity
            if (currentVelocity >= maxVelocity) {
                currentState = CONSTANT_VELOCITY;
            }
            // Check if we need to start decelerating
            else if (distanceToTarget <= stoppingDistance) {
                currentState = DECELERATING;
            }
            break;
            
        case CONSTANT_VELOCITY:
            // Check if we need to start decelerating
            if (distanceToTarget <= stoppingDistance) {
                currentState = DECELERATING;
            }
            break;
            
        case DECELERATING:
            // Stay in decelerating state until stopped
            break;
            
        case IDLE:
            // Do nothing
            break;
    }
}

float ServoAccelerationController::calculateNextPosition() {
    unsigned long currentTime = millis();
    unsigned long deltaTime = currentTime - lastUpdateTime;
    
    float newAngle = currentAngle;
    float newVelocity = currentVelocity;
    
    switch (currentState) {
        case ACCELERATING:
            // Apply acceleration
            newVelocity = currentVelocity + (accelerationRate * deltaTime);
            if (newVelocity > maxVelocity) {
                newVelocity = maxVelocity;
            }
            newAngle = currentAngle + (currentVelocity * deltaTime) + (0.5 * accelerationRate * deltaTime * deltaTime);
            break;
            
        case CONSTANT_VELOCITY:
            // Maintain constant velocity
            newAngle = currentAngle + (maxVelocity * deltaTime);
            break;
            
        case DECELERATING:
            // Apply deceleration
            newVelocity = currentVelocity - (decelerationRate * deltaTime);
            if (newVelocity < 0) {
                newVelocity = 0;
            }
            newAngle = currentAngle + (currentVelocity * deltaTime) - (0.5 * decelerationRate * deltaTime * deltaTime);
            break;
            
        case IDLE:
            // No movement
            newAngle = currentAngle;
            newVelocity = 0;
            break;
    }
    
    // Ensure we don't overshoot the target
    if ((targetAngle > currentAngle && newAngle > targetAngle) ||
        (targetAngle < currentAngle && newAngle < targetAngle)) {
        newAngle = targetAngle;
        newVelocity = 0;
    }
    
    return newAngle;
}

unsigned long ServoAccelerationController::calculateMoveTime(float startPos, float endPos) {
    float distance = abs(endPos - startPos);
    
    // Calculate time for triangular velocity profile
    float timeToMaxVel = maxVelocity / accelerationRate;
    float distanceToMaxVel = 0.5 * accelerationRate * timeToMaxVel * timeToMaxVel;
    
    if (distance <= 2 * distanceToMaxVel) {
        // Triangular profile (never reach max velocity)
        return sqrt(2.0 * distance / accelerationRate);
    } else {
        // Trapezoidal profile (reach max velocity)
        float constantVelDistance = distance - (2 * distanceToMaxVel);
        float constantVelTime = constantVelDistance / maxVelocity;
        return (2 * timeToMaxVel) + constantVelTime;
    }
}

//* ************************************************************************
//* ************************ STATUS METHODS ***************************
//* ************************************************************************

bool ServoAccelerationController::isMoving() {
    return currentState != IDLE;
}

bool ServoAccelerationController::hasReachedTarget() {
    return currentState == IDLE && abs(currentAngle - targetAngle) < 0.1;
}

float ServoAccelerationController::getCurrentAngle() {
    return currentAngle;
}

float ServoAccelerationController::getCurrentVelocity() {
    return currentVelocity;
}

ServoAccelerationController::MotionState ServoAccelerationController::getMotionState() {
    return currentState;
}

//* ************************************************************************
//* ************************ RESET METHODS ***************************
//* ************************************************************************

void ServoAccelerationController::reset() {
    currentAngle = 0.0;
    targetAngle = 0.0;
    currentVelocity = 0.0;
    startAngle = 0.0;
    currentState = IDLE;
    lastUpdateTime = millis();
    moveStartTime = millis();
}

void ServoAccelerationController::setCurrentAngle(float angle) {
    currentAngle = angle;
    targetAngle = angle;
    startAngle = angle;
    currentVelocity = 0.0;
    currentState = IDLE;
} 