#include <Arduino.h>
#include <FastAccelStepper.h>
#include <Bounce2.h>
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"
#include "ServoControl.h"

#include "CylinderControl.h"
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
// Loader height stepper motor homing and servo sequence control
// Start button triggers 2-inch down/up cycle
// OTA functionality integrated for remote updates
//!
//! ⚠️  IMPORTANT: DO NOT ADD TIMEOUT CHECKS TO THIS SYSTEM
//! ⚠️  Timeout checks were removed because they cause freezing issues
//! ⚠️  Only add timeout checks if specifically instructed to do so
//! ⚠️  The servo controller and motor have their own completion detection

//* ************************************************************************
//* *********************** MOTOR & SERVO OBJECTS *************************
//* ************************************************************************
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *loaderHeightMotor = NULL;      // Loader height stepper motor (up/down)
ServoControl loaderServo;             // Loader servo


//* ************************************************************************
//* *********************** CYLINDER OBJECTS ******************************
//* ************************************************************************
CylinderControl extensionCylinder;    // Extension cylinder control

//* ************************************************************************
//* *********************** BUTTON CONTROL ********************************
//* ************************************************************************
Bounce2::Button startButton = Bounce2::Button();
Bounce2::Button loaderHeightHomeSwitch = Bounce2::Button();

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
void initializeCylinder();
void initializeButtons();
void updateButtons();
void performStartupSequence();
void setupStateMachineReferences();
void homeLoaderHeight();
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
  initializeCylinder();
  
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
  //! STEP 4: UPDATE STATE MACHINE
  //! ************************************************************************
  updateStateMachine();

  //! ************************************************************************
  //! STEP 5: HANDLE START BUTTON PRESS
  //! ************************************************************************
  // Debug: Check start button conditions
  static unsigned long lastButtonDebugTime = 0;
  if (millis() - lastButtonDebugTime > 2000) { // Print every 2 seconds
    Serial.println("=== START BUTTON DEBUG ===");
    Serial.println("systemInitialized: " + String(systemInitialized ? "YES" : "NO"));
    Serial.println("startButton.pressed(): " + String(startButton.pressed() ? "YES" : "NO"));
    Serial.println("startButton.read(): " + String(startButton.read() ? "HIGH" : "LOW"));
    Serial.println("Current state: " + getStateName(getCurrentState()));
    Serial.println("IDLE_STATE: " + String(IDLE_STATE));
    Serial.println("==========================");
    lastButtonDebugTime = millis();
  }
  
  if (systemInitialized && startButton.pressed() && getCurrentState() == IDLE_STATE) {
    Serial.println("Start button pressed - entering TEST state");
    setState(TEST_STATE);
  }

  //! ************************************************************************
  //! STEP 6: HANDLE SERIAL COMMANDS
  //! ************************************************************************
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    // Check for manual commands first (h1.3, a30, etc.)
    if (command.length() >= 2) {
      char commandType = command.charAt(0);
      String valueStr = command.substring(1);
      float value = valueStr.toFloat();
      
      if (commandType == 'h' && value > 0 && value <= 50) {
        // Manual loader height command
        Serial.println("Manual loader height command: Moving to " + String(value) + " inches");
        if (loaderHeightMotor) {
          int targetSteps = (int)(value * STEPS_PER_INCH);
          loaderHeightMotor->setSpeedInHz(Z_MAX_SPEED);
          loaderHeightMotor->setAcceleration(Z_ACCELERATION);
          loaderHeightMotor->moveTo(targetSteps);
          Serial.println("Moving loader height to: " + String(targetSteps) + " steps (" + String(value) + " inches)");
        } else {
          Serial.println("ERROR: Loader height motor not available");
        }
        return; // Skip other command processing
      } else if (commandType == 'a' && value >= 0 && value <= 180) {
        // Manual servo angle command
        Serial.println("Manual angle command: Moving servo to " + String(value) + " degrees");
        loaderServo.write(value);
        Serial.println("Moving servo to: " + String(value) + " degrees");
        return; // Skip other command processing
      }
    }
    
    // Handle other commands
    if (command == "servo_status") {
      Serial.println("=== SERVO STATUS ===");
      Serial.println("Servo is available and ready");
    } else if (command.startsWith("servo_move ")) {
      int targetAngle = command.substring(11).toInt();
      Serial.println("Moving servo to: " + String(targetAngle) + "°");
      loaderServo.write(targetAngle);
    } else if (command == "servo_stop") {
      Serial.println("Servo stop command - no action needed (direct servo control)");
    } else if (command == "cylinder_extend") {
      Serial.println("Extending cylinder");
      extensionCylinder.extend();
    } else if (command == "cylinder_retract") {
      Serial.println("Retracting cylinder");
      extensionCylinder.retract();
    } else if (command == "cylinder_toggle") {
      Serial.println("Toggling cylinder state");
      extensionCylinder.toggle();
    } else if (command == "cylinder_status") {
      Serial.println("=== CYLINDER STATUS ===");
      Serial.println("Current state: " + String(extensionCylinder.getState() ? "EXTENDED" : "RETRACTED"));
      Serial.println("Is extended: " + String(extensionCylinder.isCylinderExtended() ? "YES" : "NO"));
    } else if (command == "test_manual") {
      Serial.println("Switching to TEST state with manual mode");
      setState(TEST_STATE);
      // The test state will handle the manual mode toggle
    } else if (command == "help") {
      Serial.println("=== AVAILABLE COMMANDS ===");
      Serial.println("h<height>  - Move loader height to height (inches) - Example: h1.3, h20");
      Serial.println("a<angle>   - Move servo to angle (degrees) - Example: a30, a90");
      Serial.println("help       - Show this help message");
      Serial.println("test_manual - Enter test state");
      Serial.println("servo_status - Show servo status");
      Serial.println("cylinder_status - Show cylinder status");
    } else if (command == "status") {
      Serial.println("=== SYSTEM STATUS ===");
      if (loaderHeightMotor) {
        int currentSteps = loaderHeightMotor->getCurrentPosition();
        float currentInches = (float)currentSteps / STEPS_PER_INCH;
        Serial.println("Loader Height Position: " + String(currentSteps) + " steps (" + String(currentInches, 2) + " inches)");
        Serial.println("Loader Height Motor running: " + String(loaderHeightMotor->isRunning() ? "YES" : "NO"));
      }
      Serial.println("Servo: Available and ready");
      Serial.println("Current State: " + getStateName(getCurrentState()));
    } else if (command.length() > 0) {
      Serial.println("Unknown command: " + command);
      Serial.println("Type 'help' for available commands");
    }
  }

  //! ************************************************************************
  //! STEP 7: SMALL DELAY TO PREVENT WATCHDOG ISSUES
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
  
  // Loader height home switch: Active HIGH (input pulldown)
  loaderHeightHomeSwitch.attach(Z_HOME_SWITCH_PIN, INPUT_PULLDOWN);
  loaderHeightHomeSwitch.interval(HOME_SWITCH_DEBOUNCE);
  
  Serial.println("Buttons and switches setup complete");
}

void initializeMotor() {
  //! ************************************************************************
  //! INITIALIZE LOADER HEIGHT STEPPER MOTOR ENGINE AND CONFIGURATION
  //! ************************************************************************
  Serial.println("Setting up loader height stepper motor...");
  
  engine.init();
  
  // Create loader height motor instance
  loaderHeightMotor = engine.stepperConnectToPin(Z_MOTOR_STEP_PIN);
  if (loaderHeightMotor) {
    loaderHeightMotor->setDirectionPin(Z_MOTOR_DIR_PIN);
    loaderHeightMotor->setSpeedInHz(Z_MAX_SPEED);
    loaderHeightMotor->setAcceleration(Z_ACCELERATION);
    loaderHeightMotor->setCurrentPosition(0);
    loaderHeightMotor->enableOutputs(); // Enable motor outputs
    
    Serial.println("Loader height motor configured successfully");
    Serial.println("Loader Height Motor speed: " + String(Z_MAX_SPEED) + " Hz");
    Serial.println("Loader Height Motor acceleration: " + String(Z_ACCELERATION) + " steps/s²");
    Serial.println("Steps per inch: " + String(STEPS_PER_INCH));
  } else {
    Serial.println("ERROR: Failed to create loader height motor instance");
  }
}

void initializeServo() {
  //! ************************************************************************
  //! INITIALIZE LOADER SERVO
  //! ************************************************************************
  Serial.println("Setting up loader servo...");
  Serial.println("Servo pin: " + String(LOADER_SERVO_PIN));
  Serial.println("Servo channel: 7");
  Serial.println("Servo frequency: 50 Hz");
  Serial.println("Servo resolution: 14 bits");
  
  loaderServo.init(LOADER_SERVO_PIN, 7, 50, 14); // Pin, channel, frequency, resolution
  
  // Set initial servo position to 90 degrees
  loaderServo.write(90);
  
  Serial.println("Loader servo initialized");
}

void initializeCylinder() {
  //! ************************************************************************
  //! INITIALIZE EXTENSION CYLINDER CONTROL
  //! ************************************************************************
  Serial.println("Setting up extension cylinder...");
  Serial.println("Cylinder pin: " + String(EXTENSION_CYLINDER_PIN));
  
  extensionCylinder.begin();
  
  // Ensure cylinder starts in retracted position
  extensionCylinder.retract();
  
  Serial.println("Extension cylinder initialized and retracted");
}

void updateButtons() {
  //! ************************************************************************
  //! UPDATE ALL BUTTON STATES
  //! ************************************************************************
  startButton.update();
  loaderHeightHomeSwitch.update();
}

void setupStateMachineReferences() {
  //! ************************************************************************
  //! SET UP REFERENCES FOR STATE MACHINE
  //! ************************************************************************
  Serial.println("Setting up state machine references...");
  
  // Set references for home state
  setHomeReferences(loaderHeightMotor, &loaderHeightHomeSwitch);

  
  // Set references for retrieve state
  setRetrieveReferences(loaderHeightMotor);
  
  // Set references for store state
  setStoreReferences(loaderHeightMotor);
  
  // Set references for test state
  setTestReferences(loaderHeightMotor, &extensionCylinder);
  setTestServoReference(&loaderServo);
  
  // Set references for idle state

  
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
  if (loaderHeightHomeSwitch.read()) {
    Serial.println("Already at home position - skipping homing");
    // Set home position without moving
    if (loaderHeightMotor) {
      loaderHeightMotor->setCurrentPosition(0);
    }
    
    // Move away from home position
    moveAwayFromHome();
    

    

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

void homeLoaderHeight() {
  //! ************************************************************************
  //! STEP 1: START HOMING SEQUENCE
  //! ************************************************************************
  Serial.println("Starting loader height homing...");
  
  if (!loaderHeightMotor) {
    Serial.println("ERROR: Loader height motor not initialized");
    return;
  }
  
  // Set homing speed
  loaderHeightMotor->setSpeedInHz(Z_HOMING_SPEED);
  
  //! ************************************************************************
  //! STEP 2: MOVE IN NEGATIVE DIRECTION UNTIL HOME SWITCH IS TRIGGERED
  //! ************************************************************************
  Serial.println("Moving toward home switch...");
  
  // Start moving in negative direction (toward home)
  loaderHeightMotor->runBackward();
  Serial.println("Motor started moving backward");
  
  // Wait until home switch is triggered (active high)
  while (!loaderHeightHomeSwitch.read()) {
    updateButtons();
    // handleOTA(); // Continue handling OTA during homing
    delay(1);
  }
  
  //! ************************************************************************
  //! STEP 3: STOP MOTOR AND SET HOME POSITION
  //! ************************************************************************
  loaderHeightMotor->forceStop();
  loaderHeightMotor->setCurrentPosition(0); // Set current position as home (0)
  
  Serial.println("Loader height homing complete - at home position");
}

void moveAwayFromHome() {
  //! ************************************************************************
  //! MOVE 2 INCHES AWAY FROM HOME POSITION
  //! ************************************************************************
  Serial.println("Moving " + String(Z_HOME_OFFSET_INCHES) + " inches away from home...");
  
  if (!loaderHeightMotor) {
    Serial.println("ERROR: Loader height motor not initialized");
    return;
  }
  
  // Debug: Print current position and target
  int currentPos = loaderHeightMotor->getCurrentPosition();
  Serial.println("Current position: " + String(currentPos) + " steps");
  Serial.println("Target position: " + String(Z_HOME_OFFSET_STEPS) + " steps");
  Serial.println("Steps per inch: " + String(STEPS_PER_INCH));
  
  // Set normal operating speed
  loaderHeightMotor->setSpeedInHz(Z_MAX_SPEED);
  Serial.println("Motor speed set to: " + String(Z_MAX_SPEED) + " Hz");
  
  // Move to the offset position (2 inches away from home)
  loaderHeightMotor->moveTo(Z_HOME_OFFSET_STEPS);
  Serial.println("Movement command sent");
  
  // Wait for movement to complete
  while (loaderHeightMotor->isRunning()) {
    delay(1);
  }
  
  int finalPos = loaderHeightMotor->getCurrentPosition();
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
  delay(1000); // Give servo time to reach position

  //! ************************************************************************
  //! STEP 2: MOVE TO 45 DEGREES
  //! ************************************************************************
  Serial.println("Moving servo to " + String(SERVO_SECOND_POS) + " degrees");
  loaderServo.write(SERVO_SECOND_POS);
  delay(1000); // Give servo time to reach position

  //! ************************************************************************
  //! STEP 3: MOVE TO 70 DEGREES
  //! ************************************************************************
  Serial.println("Moving servo to " + String(SERVO_THIRD_POS) + " degrees");
  loaderServo.write(SERVO_THIRD_POS);
  delay(1000); // Give servo time to reach position

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
  
  if (!loaderHeightMotor) {
    Serial.println("ERROR: Loader height motor not initialized");
    cycleInProgress = false;
    return;
  }
  
  // Store current position
  int currentPosition = loaderHeightMotor->getCurrentPosition();
  
  //! ************************************************************************
  //! STEP 2: MOVE DOWN 2 INCHES
  //! ************************************************************************
  Serial.println("Moving down " + String(Z_CYCLE_DISTANCE_INCHES) + " inches...");
  
  // Calculate target position (current position + 2 inches in steps)
  int downPosition = currentPosition + Z_CYCLE_DISTANCE_STEPS;
  loaderHeightMotor->moveTo(downPosition);
  
  // Wait for downward movement to complete
  while (loaderHeightMotor->isRunning()) {
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
  loaderHeightMotor->moveTo(currentPosition);
  
  // Wait for upward movement to complete
  while (loaderHeightMotor->isRunning()) {
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



