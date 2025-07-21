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
    
    // Initialize timing
    lastUpdateTime = millis();
    moveStartTime = millis();
    
    // Debug initialization
    Serial.println("ServoAccelerationController initialized");
    Serial.println("Default acceleration: " + String(accelerationRate) + " deg/s²");
    Serial.println("Default max velocity: " + String(maxVelocity) + " deg/s");
}

//* ************************************************************************
//* ************************ CONFIGURATION METHODS ***************************
//* ************************************************************************

void ServoAccelerationController::setAccelerationRate(float accelRate) {
    accelerationRate = accelRate;
    Serial.println("Acceleration rate set to: " + String(accelRate) + " deg/s²");
}

void ServoAccelerationController::setDecelerationRate(float decelRate) {
    decelerationRate = decelRate;
    Serial.println("Deceleration rate set to: " + String(decelRate) + " deg/s²");
}

void ServoAccelerationController::setMaxVelocity(float maxVel) {
    maxVelocity = maxVel;
    Serial.println("Max velocity set to: " + String(maxVel) + " deg/s");
}

void ServoAccelerationController::setAccelerationProfile(float accelRate, float maxVel) {
    accelerationRate = accelRate;
    decelerationRate = accelRate; // Always the same as acceleration
    maxVelocity = maxVel;
    Serial.println("Acceleration profile set - Accel: " + String(accelRate) + " deg/s², MaxVel: " + String(maxVel) + " deg/s");
}

//* ************************************************************************
//* ************************ MOTION CONTROL METHODS ***************************
//* ************************************************************************

void ServoAccelerationController::moveTo(float targetAngle) {
    if (!servo) {
        Serial.println("ERROR: Servo reference is NULL - cannot move");
        return;
    }
    
    this->targetAngle = targetAngle;
    startAngle = currentAngle;
    moveStartTime = millis();
    currentState = ACCELERATING;
    currentVelocity = 0.0;
    
    Serial.println("Servo moveTo: " + String(currentAngle, 1) + "° -> " + String(targetAngle, 1) + "°");
}

void ServoAccelerationController::moveToWithTime(float targetAngle, unsigned long moveTimeMs) {
    // Calculate required acceleration profile to reach target in specified time
    float distance = abs(targetAngle - currentAngle);
    
    if (distance == 0) {
        Serial.println("Already at target position");
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
        Serial.println("Already at target position");
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
    Serial.println("Servo stop command issued");
}

void ServoAccelerationController::update() {
    if (currentState == IDLE) {
        return;
    }
    
    if (!servo) {
        Serial.println("ERROR: Servo reference is NULL in update()");
        currentState = IDLE;
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
    float deltaTimeSeconds = deltaTime / 1000.0;
    currentVelocity = abs((newAngle - currentAngle) / deltaTimeSeconds); // Convert to degrees per second, always positive
    currentAngle = newAngle;
    
    // Send command to servo
    servo->write(currentAngle);
    
    // Update timing
    lastUpdateTime = currentTime;
    
    // Check if we've reached the target
    if (abs(currentAngle - targetAngle) < 0.1 && abs(currentVelocity) < 0.01) {
        currentState = IDLE;
        currentVelocity = 0.0;
        Serial.println("Servo reached target: " + String(targetAngle) + "°");
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
    unsigned long currentTime = millis();
    unsigned long deltaTime = currentTime - lastUpdateTime;
    
    // Convert deltaTime to seconds for proper acceleration calculations
    float deltaTimeSeconds = deltaTime / 1000.0;
    
    float newAngle = currentAngle;
    float newVelocity = currentVelocity;
    
    // Determine direction (1 for positive, -1 for negative)
    int direction = (targetAngle > currentAngle) ? 1 : -1;
    
    float averageVelocity = 0.0; // Declare outside switch to avoid compilation issues
    
    switch (currentState) {
        case ACCELERATING: {
            // Apply acceleration in the correct direction
            newVelocity = currentVelocity + (accelerationRate * deltaTimeSeconds);
            if (newVelocity > maxVelocity) {
                newVelocity = maxVelocity;
            }
            // Apply velocity in the correct direction (use average velocity for smooth motion)
            averageVelocity = (currentVelocity + newVelocity) / 2.0;
            newAngle = currentAngle + (direction * averageVelocity * deltaTimeSeconds);
            break;
        }
            
        case CONSTANT_VELOCITY: {
            // Maintain constant velocity in the correct direction
            newAngle = currentAngle + (direction * maxVelocity * deltaTimeSeconds);
            break;
        }
            
        case DECELERATING: {
            // Apply deceleration in the correct direction
            newVelocity = currentVelocity - (decelerationRate * deltaTimeSeconds);
            if (newVelocity < 0) {
                newVelocity = 0;
            }
            // Apply velocity in the correct direction (use average velocity for smooth motion)
            averageVelocity = (currentVelocity + newVelocity) / 2.0;
            newAngle = currentAngle + (direction * averageVelocity * deltaTimeSeconds);
            break;
        }
            
        case IDLE: {
            // No movement
            newAngle = currentAngle;
            newVelocity = 0;
            break;
        }
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
//* ************************ POSITION VERIFICATION METHODS ***************************
//* ************************************************************************

unsigned long ServoAccelerationController::getMoveCompletionTime() {
    //! ************************************************************************
    //! CALCULATE MOVE COMPLETION TIME BASED ON ACCELERATION PROFILE
    //! ************************************************************************
    
    if (currentState == IDLE) {
        return millis(); // Already complete
    }
    
    // Calculate time to reach target from current position
    float distanceToTarget = abs(targetAngle - currentAngle);
    
    if (distanceToTarget < 0.1) {
        return millis(); // Already at target
    }
    
    // Calculate time based on acceleration profile
    float accelTime = maxVelocity / accelerationRate; // Time to reach max speed
    float accelDistance = 0.5 * accelerationRate * accelTime * accelTime; // Distance covered during acceleration
    
    float totalTime;
    if (distanceToTarget <= 2 * accelDistance) {
        // Triangular profile (accelerate then decelerate)
        totalTime = 2 * sqrt(distanceToTarget / accelerationRate);
    } else {
        // Trapezoidal profile (accelerate, constant speed, decelerate)
        float constantSpeedDistance = distanceToTarget - 2 * accelDistance;
        float constantSpeedTime = constantSpeedDistance / maxVelocity;
        totalTime = 2 * accelTime + constantSpeedTime;
    }
    
    // Add 0.5 second buffer for safety
    unsigned long bufferTime = 500; // 0.5 seconds in milliseconds
    unsigned long completionTime = moveStartTime + (unsigned long)(totalTime * 1000) + bufferTime;
    
    return completionTime;
}

bool ServoAccelerationController::isMoveComplete() {
    //! ************************************************************************
    //! CHECK IF CURRENT TIME HAS PASSED THE CALCULATED COMPLETION TIME
    //! ************************************************************************
    
    if (currentState == IDLE) {
        return true; // Already complete
    }
    
    unsigned long completionTime = getMoveCompletionTime();
    return millis() >= completionTime;
}

unsigned long ServoAccelerationController::getRemainingMoveTime() {
    //! ************************************************************************
    //! CALCULATE REMAINING TIME UNTIL MOVE COMPLETION
    //! ************************************************************************
    
    if (currentState == IDLE) {
        return 0; // Already complete
    }
    
    unsigned long completionTime = getMoveCompletionTime();
    unsigned long currentTime = millis();
    
    if (currentTime >= completionTime) {
        return 0; // Move should be complete
    }
    
    return completionTime - currentTime;
}

unsigned long ServoAccelerationController::calculateMoveTimeToTarget(float targetAngle) {
    //! ************************************************************************
    //! CALCULATE TIME TO REACH SPECIFIC TARGET ANGLE FROM CURRENT POSITION
    //! ************************************************************************
    
    float distanceToTarget = abs(targetAngle - currentAngle);
    
    if (distanceToTarget < 0.1) {
        return 0; // Already at target
    }
    
    // Calculate time based on acceleration profile
    float accelTime = maxVelocity / accelerationRate; // Time to reach max speed
    float accelDistance = 0.5 * accelerationRate * accelTime * accelTime; // Distance covered during acceleration
    
    float totalTime;
    if (distanceToTarget <= 2 * accelDistance) {
        // Triangular profile (accelerate then decelerate)
        totalTime = 2 * sqrt(distanceToTarget / accelerationRate);
    } else {
        // Trapezoidal profile (accelerate, constant speed, decelerate)
        float constantSpeedDistance = distanceToTarget - 2 * accelDistance;
        float constantSpeedTime = constantSpeedDistance / maxVelocity;
        totalTime = 2 * accelTime + constantSpeedTime;
    }
    
    // Add 0.5 second buffer for safety
    unsigned long bufferTime = 500; // 0.5 seconds in milliseconds
    return (unsigned long)(totalTime * 1000) + bufferTime;
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
    lastUpdateTime = millis(); // Reset timing
} 