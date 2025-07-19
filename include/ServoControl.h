#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <Arduino.h>

class ServoControl {
private:
    int pin;
    int channel;
    int frequency;
    int resolution;
    int minPulseWidth;
    int maxPulseWidth;
    int minAngle;
    int maxAngle;
    
    int angleToDuty(float angle);
    
public:
    ServoControl();
    
    float targetAngle; // Publicly accessible target angle
    unsigned long lastUpdateTime; // Time of the last update
    unsigned long moveStartTime; // Time when movement started

    void init(int servoPin, int pwmChannel = 7, int freq = 50, int res = 14);
    void write(float angle);
    void writeMicroseconds(int microseconds);
    void detach();
    
    void setPulseWidthRange(int minUs, int maxUs);
    void setAngleRange(int minDeg, int maxDeg);
    
    bool hasReachedTarget();
    
    //! ************************************************************************
    //! MOVE COMPLETION CALCULATION METHODS
    //! ************************************************************************
    unsigned long getMoveCompletionTime();
    bool isMoveComplete();
    unsigned long getRemainingMoveTime();
    unsigned long calculateMoveTimeToTarget(int targetAngle);
    unsigned long calculateMoveTime(float startPos, float endPos);
};

#endif 