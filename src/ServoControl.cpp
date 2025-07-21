#include "ServoControl.h"

//! DO NOT CHANGE ANYTHING IN THIS FILE!!!

ServoControl::ServoControl() {
    pin = -1;
    channel = -1;
    frequency = 50;
    resolution = 14;
    minPulseWidth = 500;  // Corresponds to 0 degrees
    maxPulseWidth = 2500; // Corresponds to 180 degrees
    minAngle = 0;
    maxAngle = 180;
    moveStartTime = 0; // Initialize move start time
}

void ServoControl::init(int servoPin, int pwmChannel, int freq, int res) {
    pin = servoPin;
    channel = pwmChannel;
    frequency = freq;
    resolution = res;
    
    ledcSetup(channel, frequency, resolution);
    ledcAttachPin(pin, channel);
}

int ServoControl::angleToDuty(float angle) {
    if (angle < minAngle) angle = minAngle;
    if (angle > maxAngle) angle = maxAngle;
    
    float pulseWidth = map(angle, minAngle, maxAngle, minPulseWidth, maxPulseWidth);
    
    int maxDuty = (1 << resolution) - 1;
    int duty = (pulseWidth / (1000000.0 / frequency)) * maxDuty;
    
    return duty;
}

void ServoControl::write(float angle) {
    if (channel >= 0) {
        int duty = angleToDuty(angle);
        ledcWrite(channel, duty);
        targetAngle = angle; // Store the target angle
        lastUpdateTime = millis(); // Record the time of update
        moveStartTime = millis(); // Record when movement started
    } else {
        Serial.println("ERROR: Servo channel not initialized!");
    }
}

void ServoControl::writeMicroseconds(int microseconds) {
    if (channel >= 0) {
        int maxDuty = (1 << resolution) - 1;
        int duty = (microseconds / (1000000.0 / frequency)) * maxDuty;
        ledcWrite(channel, duty);
    }
}

void ServoControl::detach() {
    if (channel >= 0) {
        ledcDetachPin(pin);
        channel = -1;
    }
}

void ServoControl::setPulseWidthRange(int minUs, int maxUs) {
    minPulseWidth = minUs;
    maxPulseWidth = maxUs;
}

void ServoControl::setAngleRange(int minDeg, int maxDeg) {
    minAngle = minDeg;
    maxAngle = maxDeg;
} 

bool ServoControl::hasReachedTarget() {
    // Check if enough time has passed since the last write() command
    // Using a conservative 500ms delay for servo movement completion
    return millis() - lastUpdateTime >= 500;
}

//* ************************************************************************
//* ************************ MOVE COMPLETION CALCULATION METHODS ***************************
//* ************************************************************************

unsigned long ServoControl::calculateMoveTime(float startPos, float endPos) {
    //! ************************************************************************
    //! CALCULATE MOVE TIME BASED ON SERVO SPEED AND DISTANCE
    //! ************************************************************************
    
    float distance = abs(endPos - startPos);
    
    if (distance < 0.1) {
        return 0; // Already at target
    }
    
    // Standard servo speed is approximately 120 degrees per second (faster than conservative estimate)
    // This provides more accurate timing for modern servos
    float servoSpeed = 120.0; // degrees per second
    float moveTimeSeconds = distance / servoSpeed;
    
    // Convert to milliseconds and add smaller safety buffer
    unsigned long moveTimeMs = (unsigned long)(moveTimeSeconds * 1000);
    unsigned long safetyBuffer = 10; // 10ms safety buffer (reduced from 20ms)
    
    return moveTimeMs + safetyBuffer;
}

unsigned long ServoControl::getMoveCompletionTime() {
    //! ************************************************************************
    //! CALCULATE MOVE COMPLETION TIME BASED ON SERVO SPEED
    //! ************************************************************************
    
    // Calculate time to reach target from current position
    float distanceToTarget = abs(targetAngle - 90.0); // Assume starting from center (90 degrees)
    
    if (distanceToTarget < 0.1) {
        return millis(); // Already at target
    }
    
    // Calculate move time based on servo speed
    unsigned long moveTime = calculateMoveTime(90.0, targetAngle);
    unsigned long completionTime = moveStartTime + moveTime;
    
    return completionTime;
}

bool ServoControl::isMoveComplete() {
    //! ************************************************************************
    //! CHECK IF CURRENT TIME HAS PASSED THE CALCULATED COMPLETION TIME
    //! ************************************************************************
    
    unsigned long completionTime = getMoveCompletionTime();
    return millis() >= completionTime;
}

unsigned long ServoControl::getRemainingMoveTime() {
    //! ************************************************************************
    //! CALCULATE REMAINING TIME UNTIL MOVE COMPLETION
    //! ************************************************************************
    
    unsigned long completionTime = getMoveCompletionTime();
    unsigned long currentTime = millis();
    
    if (currentTime >= completionTime) {
        return 0; // Move should be complete
    }
    
    return completionTime - currentTime;
}

unsigned long ServoControl::calculateMoveTimeToTarget(int targetAngle) {
    //! ************************************************************************
    //! CALCULATE TIME TO REACH SPECIFIC TARGET ANGLE FROM CURRENT POSITION
    //! ************************************************************************
    
    float distanceToTarget = abs((float)targetAngle - 90.0); // Assume starting from center
    
    if (distanceToTarget < 0.1) {
        return 0; // Already at target
    }
    
    return calculateMoveTime(90.0, (float)targetAngle);
} 