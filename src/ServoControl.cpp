#include "ServoControl.h"

ServoControl::ServoControl() {
    pin = -1;
    channel = -1;
    frequency = 50;
    resolution = 16;
    minPulseWidth = 500;
    maxPulseWidth = 2500;
    minAngle = 0;
    maxAngle = 180;
}

void ServoControl::init(int servoPin, int pwmChannel, int freq, int res) {
    pin = servoPin;
    channel = pwmChannel;
    frequency = freq;
    resolution = res;
    
    // Setup PWM channel
    ledcSetup(channel, frequency, resolution);
    ledcAttachPin(pin, channel);
    
    Serial.println("Servo initialized:");
    Serial.println("  Pin: " + String(pin));
    Serial.println("  Channel: " + String(channel));
    Serial.println("  Frequency: " + String(frequency) + " Hz");
    Serial.println("  Resolution: " + String(resolution) + " bits");
}

int ServoControl::angleToDuty(float angle) {
    // Constrain angle to valid range
    if (angle < minAngle) angle = minAngle;
    if (angle > maxAngle) angle = maxAngle;
    
    // Map angle to pulse width in microseconds using linear interpolation
    float pulseWidth = minPulseWidth + (angle - minAngle) * (maxPulseWidth - minPulseWidth) / (maxAngle - minAngle);
    
    // Convert pulse width to duty cycle
    // Period = 1/frequency seconds = 1,000,000/frequency microseconds
    float periodMicros = 1000000.0 / frequency;
    
    // Calculate duty cycle as percentage of period
    float dutyCyclePercent = pulseWidth / periodMicros;
    
    // Convert to actual duty value based on resolution
    int maxDuty = (1 << resolution) - 1;
    int duty = (int)(dutyCyclePercent * maxDuty);
    
    return duty;
}

void ServoControl::write(float angle) {
    if (channel >= 0) {
        int duty = angleToDuty(angle);
        ledcWrite(channel, duty);
        
        Serial.println("Servo moving to " + String(angle) + "° (duty: " + String(duty) + ")");
    } else {
        Serial.println("ERROR: Servo not initialized");
    }
}

void ServoControl::writeMicroseconds(int microseconds) {
    if (channel >= 0) {
        // Convert microseconds to duty cycle
        float periodMicros = 1000000.0 / frequency;
        float dutyCyclePercent = microseconds / periodMicros;
        int maxDuty = (1 << resolution) - 1;
        int duty = (int)(dutyCyclePercent * maxDuty);
        
        ledcWrite(channel, duty);
        
        Serial.println("Servo pulse width: " + String(microseconds) + "μs (duty: " + String(duty) + ")");
    } else {
        Serial.println("ERROR: Servo not initialized");
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