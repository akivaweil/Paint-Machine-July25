#include <Arduino.h>
#include <FastAccelStepper.h>
#include <Bounce2.h>
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"
#include "ServoControl.h"
#include "ServoAccelerationController.h"
#include "OTA_Manager.h"
#include "StateMachine.h"

//* ************************************************************************
//* *********************** OTA MANAGER ***********************************
//* ************************************************************************
void initOTA();
void handleOTA();

//* ************************************************************************
//* *********************** PAINT MACHINE LOADER **************************
//* ************************************************************************
// Paint Machine Loader Control System with OTA Support
// Z-axis stepper motor homing and servo sequence control
// Start button triggers 2-inch down/up cycle
// OTA functionality integrated for remote updates

//* ************************************************************************
//* *********************** MOTOR & SERVO OBJECTS *************************
//* ************************************************************************
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *zMotor = NULL;      // Z-axis stepper motor (up/down)
ServoControl loaderServo;             // Loader servo
ServoAccelerationController mainServoController(&loaderServo); // Servo acceleration controller

//* ************************************************************************
//* *********************** BUTTON CONTROL ********************************
//* ************************************************************************
Bounce2::Button startButton = Bounce2::Button();
Bounce2::Button zHomeSwitch = Bounce2::Button();

//* ************************************************************************
//* *********************** STATE VARIABLES *******************************
//* ************************************************************************
bool systemInitialized = false;      // Flag to track if startup sequence is complete
bool cycleInProgress = false;         // Flag to prevent multiple cycles running simultaneously

//* ************************************************************************
//* *********************** FUNCTION DECLARATIONS *************************
//* ************************************************************************
void initializeMotor();
void initializeServo();
void initializeButtons();
void updateButtons();
void performStartupSequence();
void setupStateMachineReferences();
void homeZAxis();
void moveAwayFromHome();
void performServoSequence();
void performCycle();
void performServoAccelerationSequence();

//* ************************************************************************
//* *********************** SETUP FUNCTION ********************************
//* ************************************************************************
void setup() {
  //! ************************************************************************
  //! STEP 1: INITIALIZE SERIAL COMMUNICATION
  //! ************************************************************************
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== PAINT MACHINE CONTROL SYSTEM STARTING ===");

  //! ************************************************************************
  //! STEP 2: INITIALIZE OTA FUNCTIONALITY
  //! ************************************************************************
  Serial.println("Initializing OTA...");
  initOTA();
  Serial.println("OTA initialization complete");

  //! ************************************************************************
  //! STEP 3: INITIALIZE HARDWARE SYSTEMS
  //! ************************************************************************
  Serial.println("Initializing hardware systems...");
  
  initializeButtons();
  initializeMotor();
  initializeServo();
  
  Serial.println("Hardware systems initialized");

  //! ************************************************************************
  //! STEP 4: INITIALIZE STATE MACHINE
  //! ************************************************************************
  Serial.println("Initializing state machine...");
  initStateMachine();
  
  //! ************************************************************************
  //! STEP 5: PERFORM STARTUP SEQUENCE
  //! ************************************************************************
  Serial.println("Starting startup sequence...");
  performStartupSequence();
  
  Serial.println("=== PAINT MACHINE LOADER READY ===");
}

//* ************************************************************************
//* *********************** MAIN LOOP *************************************
//* ************************************************************************
void loop() {
  //! ************************************************************************
  //! STEP 1: HANDLE OTA UPDATES
  //! ************************************************************************
    handleOTA();


  //! ************************************************************************
  //! STEP 2: UPDATE BUTTON STATES
  //! ************************************************************************
  updateButtons();

  //! ************************************************************************
  //! STEP 3: UPDATE STATE MACHINE
  //! ************************************************************************
  updateStateMachine();

  //! ************************************************************************
  //! STEP 4: HANDLE START BUTTON PRESS
  //! ************************************************************************
  if (systemInitialized && startButton.pressed() && getCurrentState() == IDLE_STATE) {
    Serial.println("Start button pressed - entering TEST state");
    setState(TEST_STATE);
  }

  //! ************************************************************************
  //! STEP 4: SMALL DELAY TO PREVENT WATCHDOG ISSUES
  //! ************************************************************************
  delay(10);
}

//* ************************************************************************
//* *********************** FUNCTION DEFINITIONS **************************
//* ************************************************************************

void initializeButtons() {
  //! ************************************************************************
  //! INITIALIZE BUTTONS AND SWITCHES WITH PROPER INPUT MODES
  //! ************************************************************************
  Serial.println("Setting up buttons and switches...");
  
  // Start button: Active HIGH (input pulldown)
  startButton.attach(START_BUTTON_PIN, INPUT_PULLDOWN);
  startButton.interval(START_BUTTON_DEBOUNCE);
  
  // Z home switch: Active HIGH (input pulldown)
  zHomeSwitch.attach(Z_HOME_SWITCH_PIN, INPUT_PULLDOWN);
  zHomeSwitch.interval(HOME_SWITCH_DEBOUNCE);
  
  Serial.println("Buttons and switches setup complete");
}

void initializeMotor() {
  //! ************************************************************************
  //! INITIALIZE Z-AXIS STEPPER MOTOR ENGINE AND CONFIGURATION
  //! ************************************************************************
  Serial.println("Setting up Z-axis stepper motor...");
  
  engine.init();
  
  // Create Z-axis motor instance
  zMotor = engine.stepperConnectToPin(Z_MOTOR_STEP_PIN);
  if (zMotor) {
    zMotor->setDirectionPin(Z_MOTOR_DIR_PIN);
    zMotor->setSpeedInHz(Z_MAX_SPEED);
    zMotor->setAcceleration(Z_ACCELERATION);
    zMotor->setCurrentPosition(0);
    zMotor->enableOutputs(); // Enable motor outputs
    
    Serial.println("Z-axis motor configured successfully");
    Serial.println("Z Motor speed: " + String(Z_MAX_SPEED) + " Hz");
    Serial.println("Z Motor acceleration: " + String(Z_ACCELERATION) + " steps/s²");
    Serial.println("Steps per inch: " + String(STEPS_PER_INCH));
  } else {
    Serial.println("ERROR: Failed to create Z-axis motor instance");
  }
}

void initializeServo() {
  //! ************************************************************************
  //! INITIALIZE LOADER SERVO AND ACCELERATION CONTROLLER
  //! ************************************************************************
  Serial.println("Setting up loader servo...");
  Serial.println("Servo pin: " + String(LOADER_SERVO_PIN));
  Serial.println("Servo channel: 7");
  Serial.println("Servo frequency: 50 Hz");
  Serial.println("Servo resolution: 14 bits");
  
  loaderServo.init(LOADER_SERVO_PIN, 7, 50, 14); // Pin, channel, frequency, resolution
  
  // Configure servo acceleration controller with conservative default settings
  mainServoController.setAccelerationProfile(
    10.0,   // Base acceleration rate (degrees/second^2)
    10.0,   // Deceleration rate (degrees/second^2)
    30.0    // Max velocity (degrees/second)
  );
  
  Serial.println("Loader servo and acceleration controller initialized");
}

void updateButtons() {
  //! ************************************************************************
  //! UPDATE ALL BUTTON STATES
  //! ************************************************************************
  startButton.update();
  zHomeSwitch.update();
}

void setupStateMachineReferences() {
  //! ************************************************************************
  //! SET UP REFERENCES FOR STATE MACHINE
  //! ************************************************************************
  Serial.println("Setting up state machine references...");
  
  // Set references for home state
  setHomeReferences(zMotor, &zHomeSwitch);
  setHomeServoReferences(&loaderServo, &mainServoController);
  
  // Set references for retrieve state
  setRetrieveReferences(zMotor);
  
  // Set references for store state
  setStoreReferences(zMotor);
  
  // Set references for test state
  setTestReferences(zMotor);
  
  Serial.println("State machine references configured");
}

void performStartupSequence() {
  //! ************************************************************************
  //! PERFORM COMPLETE STARTUP SEQUENCE
  //! ************************************************************************
  Serial.println("=== STARTING STARTUP SEQUENCE ===");
  
  // Step 1: Set up state machine references
  setupStateMachineReferences();
  
  // Step 2: Check if already at home position
  updateButtons(); // Update button states
  if (zHomeSwitch.read()) {
    Serial.println("Already at home position - skipping homing");
    // Set home position without moving
    if (zMotor) {
      zMotor->setCurrentPosition(0);
    }
    
    // Move away from home position
    moveAwayFromHome();
    
    // Perform servo acceleration sequence
    Serial.println("Performing servo acceleration sequence...");
    performServoAccelerationSequence();
  } else {
    // Step 3: Start home state (which now includes moving away from home)
    setState(HOME_STATE);
    
    // Step 4: Wait for homing to complete
    while (getCurrentState() == HOME_STATE) {
      updateStateMachine();
      updateButtons();
      delay(10);
    }
  }
  
  systemInitialized = true;
  Serial.println("=== STARTUP SEQUENCE COMPLETE ===");
}

//* ************************************************************************
//* ************************ HOMING ***************************
//* ************************************************************************
// Homes the Z-axis by moving toward the home switch until it's triggered

void homeZAxis() {
  //! ************************************************************************
  //! STEP 1: START HOMING SEQUENCE
  //! ************************************************************************
  Serial.println("Starting Z-axis homing...");
  
  if (!zMotor) {
    Serial.println("ERROR: Z motor not initialized");
    return;
  }
  
  // Set homing speed
  zMotor->setSpeedInHz(Z_HOMING_SPEED);
  
  //! ************************************************************************
  //! STEP 2: MOVE IN NEGATIVE DIRECTION UNTIL HOME SWITCH IS TRIGGERED
  //! ************************************************************************
  Serial.println("Moving toward home switch...");
  
  // Start moving in negative direction (toward home)
  zMotor->runBackward();
  Serial.println("Motor started moving backward");
  
  // Wait until home switch is triggered (active high)
  int homingTimeout = 0;
  while (!zHomeSwitch.read() && homingTimeout < 30000) { // 30 second timeout
    updateButtons();
    // handleOTA(); // Continue handling OTA during homing
    delay(1);
    homingTimeout++;
  }
  
  if (homingTimeout >= 30000) {
    Serial.println("ERROR: Homing timeout - home switch not triggered");
    zMotor->forceStop();
    return;
  }
  
  //! ************************************************************************
  //! STEP 3: STOP MOTOR AND SET HOME POSITION
  //! ************************************************************************
  zMotor->forceStop();
  zMotor->setCurrentPosition(0); // Set current position as home (0)
  
  Serial.println("Z-axis homing complete - at home position");
}

void moveAwayFromHome() {
  //! ************************************************************************
  //! MOVE 2 INCHES AWAY FROM HOME POSITION
  //! ************************************************************************
  Serial.println("Moving " + String(Z_HOME_OFFSET_INCHES) + " inches away from home...");
  
  if (!zMotor) {
    Serial.println("ERROR: Z motor not initialized");
    return;
  }
  
  // Debug: Print current position and target
  int currentPos = zMotor->getCurrentPosition();
  Serial.println("Current position: " + String(currentPos) + " steps");
  Serial.println("Target position: " + String(Z_HOME_OFFSET_STEPS) + " steps");
  Serial.println("Steps per inch: " + String(STEPS_PER_INCH));
  
  // Set normal operating speed
  zMotor->setSpeedInHz(Z_MAX_SPEED);
  Serial.println("Motor speed set to: " + String(Z_MAX_SPEED) + " Hz");
  
  // Move to the offset position (2 inches away from home)
  zMotor->moveTo(Z_HOME_OFFSET_STEPS);
  Serial.println("Movement command sent");
  
  // Wait for movement to complete
  int timeout = 0;
  while (zMotor->isRunning() && timeout < 10000) { // 10 second timeout
    delay(1);
    timeout++;
  }
  
  if (timeout >= 10000) {
    Serial.println("WARNING: Movement timeout - motor may be stuck");
  }
  
  int finalPos = zMotor->getCurrentPosition();
  Serial.println("Final position: " + String(finalPos) + " steps");
  Serial.println("Moved to position: " + String(Z_HOME_OFFSET_INCHES) + " inches from home");
}

void performServoSequence() {
  //! ************************************************************************
  //! PERFORM SERVO SEQUENCE: 90° -> 45° -> 70°
  //! ************************************************************************
  Serial.println("Starting servo sequence...");

  //! ************************************************************************
  //! STEP 1: MOVE TO 90 DEGREES
  //! ************************************************************************
  Serial.println("Moving servo to " + String(SERVO_START_POS) + " degrees");
  loaderServo.write(SERVO_START_POS);
  while (!loaderServo.hasReachedTarget()) {
    // handleOTA();
    delay(10);
  }

  //! ************************************************************************
  //! STEP 2: MOVE TO 45 DEGREES
  //! ************************************************************************
  Serial.println("Moving servo to " + String(SERVO_SECOND_POS) + " degrees");
  loaderServo.write(SERVO_SECOND_POS);
  while (!loaderServo.hasReachedTarget()) {
    // handleOTA();
    delay(10);
  }

  //! ************************************************************************
  //! STEP 3: MOVE TO 70 DEGREES
  //! ************************************************************************
  Serial.println("Moving servo to " + String(SERVO_THIRD_POS) + " degrees");
  loaderServo.write(SERVO_THIRD_POS);
  while (!loaderServo.hasReachedTarget()) {
    // handleOTA();
    delay(10);
  }

  Serial.println("Servo sequence complete");
}

//* ************************************************************************
//* ************************ CYCLE OPERATION ******************************
//* ************************************************************************
// Performs down 2 inches, then up 2 inches cycle when start button is pressed

void performCycle() {
  //! ************************************************************************
  //! STEP 1: SET CYCLE IN PROGRESS FLAG
  //! ************************************************************************
  cycleInProgress = true;
  Serial.println("=== STARTING CYCLE OPERATION ===");
  
  if (!zMotor) {
    Serial.println("ERROR: Z motor not initialized");
    cycleInProgress = false;
    return;
  }
  
  // Store current position
  int currentPosition = zMotor->getCurrentPosition();
  
  //! ************************************************************************
  //! STEP 2: MOVE DOWN 2 INCHES
  //! ************************************************************************
  Serial.println("Moving down " + String(Z_CYCLE_DISTANCE_INCHES) + " inches...");
  
  // Calculate target position (current position + 2 inches in steps)
  int downPosition = currentPosition + Z_CYCLE_DISTANCE_STEPS;
  zMotor->moveTo(downPosition);
  
  // Wait for downward movement to complete
  while (zMotor->isRunning()) {
    updateButtons(); // Continue monitoring buttons
    // handleOTA(); // Continue handling OTA during movement
    delay(1);
  }
  
  Serial.println("Reached down position");
  
  //! ************************************************************************
  //! STEP 3: MOVE UP 2 INCHES (BACK TO ORIGINAL POSITION)
  //! ************************************************************************
  Serial.println("Moving up " + String(Z_CYCLE_DISTANCE_INCHES) + " inches...");
  
  // Move back to original position
  zMotor->moveTo(currentPosition);
  
  // Wait for upward movement to complete
  while (zMotor->isRunning()) {
    updateButtons(); // Continue monitoring buttons
    // handleOTA(); // Continue handling OTA during movement
    delay(1);
  }
  
  Serial.println("Returned to original position");
  
  //! ************************************************************************
  //! STEP 4: CYCLE COMPLETE
  //! ************************************************************************
  cycleInProgress = false;
  Serial.println("=== CYCLE OPERATION COMPLETE ===");
}

void performServoAccelerationSequence() {
  //! ************************************************************************
  //! PERFORM SERVO ACCELERATION SEQUENCE: 5 movements with increasing acceleration
  //! ************************************************************************
  Serial.println("Starting servo acceleration sequence...");
  
  // Set initial servo position to center (90 degrees)
  mainServoController.setCurrentAngle(90.0);
  loaderServo.write(90.0);
  delay(100); // Give servo time to reach position
  
  // Perform 5 movements with increasing acceleration
  for (int movement = 0; movement < 5; movement++) {
    // Calculate acceleration for this movement (increasing with each movement)
    float baseAccel = 5.0; // Base acceleration rate (degrees/second^2)
    float currentAccel = baseAccel * (movement + 1); // Increase acceleration each time
    
    // Set acceleration profile for this movement
    mainServoController.setAccelerationProfile(
      currentAccel,  // Acceleration rate
      currentAccel,  // Deceleration rate (same as acceleration)
      currentAccel * 3  // Max velocity (calculated from acceleration)
    );
    
    // Calculate target angle (alternate between +30 and -30 degrees from center)
    float targetAngle;
    if (movement % 2 == 0) {
      targetAngle = 90.0 + 30.0; // Move to 120 degrees
    } else {
      targetAngle = 90.0 - 30.0; // Move to 60 degrees
    }
    
    // Start the movement
    mainServoController.moveTo(targetAngle);
    
    Serial.println("Servo movement " + String(movement + 1) + 
                  " - Moving to " + String(targetAngle) + 
                  " degrees with acceleration " + String(currentAccel));
    
    // Wait for movement to complete
    while (mainServoController.isMoving()) {
      mainServoController.update();
      delay(10);
    }
  }
  
  Serial.println("Servo acceleration sequence complete");
}