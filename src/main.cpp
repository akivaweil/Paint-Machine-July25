#include <Arduino.h>
#include <FastAccelStepper.h>
#include <Bounce2.h>
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"
#include "ServoControl.h"

//* ************************************************************************
//* *********************** PAINT MACHINE LOADER **************************
//* ************************************************************************
// Paint Machine Loader Control System with OTA Support
// Z-axis stepper motor homing and servo sequence control
// Start button triggers 2-inch down/up cycle
// OTA functionality integrated for remote updates

// External OTA functions
extern void setupOTA();
extern void handleOTA();

//* ************************************************************************
//* *********************** MOTOR & SERVO OBJECTS *************************
//* ************************************************************************
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *zMotor = NULL;      // Z-axis stepper motor (up/down)
ServoControl loaderServo;             // Loader servo

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
void homeZAxis();
void moveAwayFromHome();
void performServoSequence();
void performCycle();

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
  setupOTA();
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
  //! STEP 4: PERFORM STARTUP SEQUENCE
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
  //! STEP 3: HANDLE START BUTTON PRESS
  //! ************************************************************************
  if (systemInitialized && startButton.pressed() && !cycleInProgress) {
    Serial.println("Start button pressed - beginning cycle");
    performCycle();
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
  zHomeSwitch.attach(Z_HOME_SWITCH_PIN, INPUT);
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
  //! INITIALIZE LOADER SERVO
  //! ************************************************************************
  Serial.println("Setting up loader servo...");
  
  loaderServo.init(LOADER_SERVO_PIN, 0, 50, 16); // Pin, channel, frequency, resolution
  
  //! ************************************************************************
  //! TEST SERVO MOVEMENT
  //! ************************************************************************
  Serial.println("Testing servo movement...");
  
  // Test movement to center position
  Serial.println("Moving to 90 degrees (center)");
  loaderServo.write(90);
  delay(1000);
  
  // Test movement to one extreme
  Serial.println("Moving to 0 degrees");
  loaderServo.write(0);
  delay(1000);
  
  // Test movement to other extreme
  Serial.println("Moving to 180 degrees");
  loaderServo.write(180);
  delay(1000);
  
  // Return to center
  Serial.println("Returning to 90 degrees");
  loaderServo.write(90);
  delay(500);
  
  Serial.println("Servo test complete - servo initialized");
}

void updateButtons() {
  //! ************************************************************************
  //! UPDATE ALL BUTTON STATES
  //! ************************************************************************
  startButton.update();
  zHomeSwitch.update();
}

void performStartupSequence() {
  //! ************************************************************************
  //! PERFORM COMPLETE STARTUP SEQUENCE
  //! ************************************************************************
  Serial.println("=== STARTING STARTUP SEQUENCE ===");
  
  // Step 1: Home the Z-axis
  homeZAxis();
  
  // Step 2: Move 10 inches away from home
  moveAwayFromHome();
  
  // Step 3: Perform servo sequence
  performServoSequence();
  
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
  
  // Wait until home switch is triggered (active high)
  while (!zHomeSwitch.read()) {
    updateButtons();
    handleOTA(); // Continue handling OTA during homing
    delay(1);
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
  //! MOVE 10 INCHES AWAY FROM HOME POSITION
  //! ************************************************************************
  Serial.println("Moving 10 inches away from home...");
  
  if (!zMotor) {
    Serial.println("ERROR: Z motor not initialized");
    return;
  }
  
  // Set normal operating speed
  zMotor->setSpeedInHz(Z_MAX_SPEED);
  
  // Move to the offset position (10 inches away from home)
  zMotor->moveTo(Z_HOME_OFFSET_STEPS);
  
  // Wait for movement to complete
  while (zMotor->isRunning()) {
    handleOTA(); // Continue handling OTA during movement
    delay(1);
  }
  
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
  delay(SERVO_MOVE_DELAY);
  
  //! ************************************************************************
  //! STEP 2: MOVE TO 45 DEGREES
  //! ************************************************************************
  Serial.println("Moving servo to " + String(SERVO_SECOND_POS) + " degrees");
  loaderServo.write(SERVO_SECOND_POS);
  delay(SERVO_MOVE_DELAY);
  
  //! ************************************************************************
  //! STEP 3: MOVE TO 70 DEGREES
  //! ************************************************************************
  Serial.println("Moving servo to " + String(SERVO_THIRD_POS) + " degrees");
  loaderServo.write(SERVO_THIRD_POS);
  delay(SERVO_MOVE_DELAY);
  
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
    handleOTA(); // Continue handling OTA during movement
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
    handleOTA(); // Continue handling OTA during movement
    delay(1);
  }
  
  Serial.println("Returned to original position");
  
  //! ************************************************************************
  //! STEP 4: CYCLE COMPLETE
  //! ************************************************************************
  cycleInProgress = false;
  Serial.println("=== CYCLE OPERATION COMPLETE ===");
}