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
    accelerationRate = 10.0; // 10 degrees per second^2
    decelerationRate = 10.0; // 10 degrees per second^2
    maxVelocity = 30.0; // 30 degrees per second
    
    // Initialize motion state
    currentAngle = 90.0; // Start at 90 degrees (middle position)
    targetAngle = 90.0;
    currentVelocity = 0.0;
    startAngle = 90.0;
    currentState = IDLE;
    
    // Initialize timing with microsecond precision
    lastUpdateTime = micros();
    moveStartTime = micros();
    
    // Position smoothing variables
    smoothedAngle = 90.0;
    smoothingFactor = 0.1; // Smoothing factor (0.1 = 10% of new position per update)
    minPositionIncrement = 0.1; // Minimum position increment to prevent stuttering
    
    // Update frequency control
    updateInterval = 10000; // 10ms update interval (100Hz) for smooth motion
    lastServoUpdateTime = 0;
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

void ServoAccelerationController::setAccelerationProfile(float accelRate, float maxVel) {
    accelerationRate = accelRate;
    decelerationRate = accelRate; // Always the same as acceleration
    maxVelocity = maxVel;
}

void ServoAccelerationController::setSmoothingFactor(float factor) {
    // Clamp smoothing factor between 0.01 and 1.0
    smoothingFactor = constrain(factor, 0.01, 1.0);
}

void ServoAccelerationController::setUpdateInterval(unsigned long intervalMicros) {
    updateInterval = intervalMicros;
}

//* ************************************************************************
//* ************************ MOTION CONTROL METHODS ***************************
//* ************************************************************************

void ServoAccelerationController::moveTo(float targetAngle) {
    this->targetAngle = targetAngle;
    startAngle = currentAngle;
    moveStartTime = micros();
    currentState = ACCELERATING;
    currentVelocity = 0.0;
    
    // Reset smoothing when starting new movement
    smoothedAngle = currentAngle;
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
    setAccelerationProfile(requiredAccel, requiredAccel * moveTimeMs / 2.0);
    
    // Start the move
    moveTo(targetAngle);
}

void ServoAccelerationController::moveToWithCurve(float targetAngle, int accelerationCurve) {
    // Clamp acceleration curve to 0-100 range
    accelerationCurve = constrain(accelerationCurve, 0, 100);
    
    // Calculate distance to target
    float distance = abs(targetAngle - currentAngle);
    
    if (distance == 0) {
        return; // Already at target
    }
    
    // Convert curve (0-100) to acceleration parameters
    // 0 = very slow and smooth, 100 = very fast and aggressive
    float baseAccelRate = 5.0;   // Base acceleration rate (degrees/second^2)
    float maxAccelRate = 50.0;   // Maximum acceleration rate (degrees/second^2)
    float baseMaxVel = 15.0;     // Base maximum velocity (degrees/second)
    float maxMaxVel = 60.0;      // Maximum velocity (degrees/second)
    
    // Calculate acceleration rate based on curve
    float accelRate = baseAccelRate + (accelerationCurve / 100.0f) * (maxAccelRate - baseAccelRate);
    
    // Calculate maximum velocity based on curve
    float maxVel = baseMaxVel + (accelerationCurve / 100.0f) * (maxMaxVel - baseMaxVel);
    
    // Deceleration rate is typically the same as acceleration for smooth motion
    float decelRate = accelRate;
    
    // Set the acceleration profile
    setAccelerationProfile(accelRate, maxVel);
    
    // Start the move
    moveTo(targetAngle);
}

void ServoAccelerationController::stop() {
    currentState = DECELERATING;
    targetAngle = currentAngle; // Stop at current position
}

void ServoAccelerationController::update() {
    unsigned long currentTime = micros();
    
    // Check if enough time has passed for an update
    if (currentTime - lastUpdateTime < updateInterval) {
        return;
    }
    
    if (currentState == IDLE) {
        return;
    }
    
    unsigned long deltaTime = currentTime - lastUpdateTime;
    
    if (deltaTime == 0) {
        return; // No time has passed
    }
    
    // Update motion state
    updateMotionState();
    
    // Calculate new position
    float newAngle = calculateNextPosition();
    
    // Apply position smoothing to eliminate "notch" behavior
    smoothedAngle = smoothedAngle + (smoothingFactor * (newAngle - smoothedAngle));
    
    // Ensure minimum position increment to prevent stuttering
    float positionDiff = abs(smoothedAngle - currentAngle);
    if (positionDiff < minPositionIncrement && currentState != IDLE) {
        // Apply minimum increment in the correct direction
        int direction = (smoothedAngle > currentAngle) ? 1 : -1;
        smoothedAngle = currentAngle + (direction * minPositionIncrement);
    }
    
    // Update current angle and velocity
    float deltaTimeSeconds = deltaTime / 1000000.0; // Convert microseconds to seconds
    currentVelocity = abs((smoothedAngle - currentAngle) / deltaTimeSeconds);
    currentAngle = smoothedAngle;
    
    // Send command to servo at controlled frequency
    if (currentTime - lastServoUpdateTime >= 5000) { // 5ms servo update rate (200Hz)
        servo->write(currentAngle);
        lastServoUpdateTime = currentTime;
    }
    
    // Update timing
    lastUpdateTime = currentTime;
    
    // Check if we've reached the target
    if (abs(currentAngle - targetAngle) < 0.1 && abs(currentVelocity) < 0.01) {
        currentState = IDLE;
        currentVelocity = 0.0;
        smoothedAngle = targetAngle; // Ensure final position is exact
        servo->write(targetAngle); // Send final position command
    }
}

//* ************************************************************************
//* ************************ PRIVATE CALCULATION METHODS ***************************
//* ************************************************************************

void ServoAccelerationController::updateMotionState() {
    float distanceToTarget = abs(targetAngle - currentAngle);
    float distanceFromStart = abs(currentAngle - startAngle);
    
    // Calculate stopping distance with current velocity
    // Note: decelerationRate is in degrees/second^2, so this calculation is correct
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
    unsigned long currentTime = micros();
    unsigned long deltaTime = currentTime - lastUpdateTime;
    
    // Convert deltaTime to seconds for proper acceleration calculations
    float deltaTimeSeconds = deltaTime / 1000000.0; // Convert microseconds to seconds
    
    float newAngle = currentAngle;
    float newVelocity = currentVelocity;
    
    // Determine direction (1 for positive, -1 for negative)
    int direction = (targetAngle > currentAngle) ? 1 : -1;
    
    switch (currentState) {
        case ACCELERATING:
            // Apply acceleration in the correct direction
            newVelocity = currentVelocity + (accelerationRate * deltaTimeSeconds);
            if (newVelocity > maxVelocity) {
                newVelocity = maxVelocity;
            }
            // Apply velocity in the correct direction
            newAngle = currentAngle + (direction * currentVelocity * deltaTimeSeconds) + 
                      (0.5 * direction * accelerationRate * deltaTimeSeconds * deltaTimeSeconds);
            break;
            
        case CONSTANT_VELOCITY:
            // Maintain constant velocity in the correct direction
            newAngle = currentAngle + (direction * maxVelocity * deltaTimeSeconds);
            break;
            
        case DECELERATING:
            // Apply deceleration in the correct direction
            newVelocity = currentVelocity - (decelerationRate * deltaTimeSeconds);
            if (newVelocity < 0) {
                newVelocity = 0;
            }
            // Apply velocity in the correct direction
            newAngle = currentAngle + (direction * currentVelocity * deltaTimeSeconds) - 
                      (0.5 * direction * decelerationRate * deltaTimeSeconds * deltaTimeSeconds);
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
    lastUpdateTime = micros();
    moveStartTime = micros();
    
    // Reset smoothing
    smoothedAngle = 90.0;
}

void ServoAccelerationController::setCurrentAngle(float angle) {
    currentAngle = angle;
    targetAngle = angle;
    startAngle = angle;
    currentVelocity = 0.0;
    currentState = IDLE;
    lastUpdateTime = micros(); // Reset timing
    
    // Reset smoothing
    smoothedAngle = angle;
} 