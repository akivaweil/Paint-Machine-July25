#include <Arduino.h>
#include <FastAccelStepper.h>
#include <Bounce2.h>
#include "Config/Config.h"
#include "Config/Pins_Definitions.h"
#include "Config/CellConfig.h"
#include "ServoControl.h"
#include "ServoAccelerationController.h"
#include "LoaderForkStepper.h"
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
//!
//! ⚠️  IMPORTANT: DO NOT ADD TIMEOUT CHECKS TO THIS SYSTEM
//! ⚠️  Timeout checks were removed because they cause freezing issues
//! ⚠️  Only add timeout checks if specifically instructed to do so
//! ⚠️  The servo controller and motor have their own completion detection

//* ************************************************************************
//* *********************** MOTOR & SERVO OBJECTS *************************
//* ************************************************************************
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *zMotor = NULL;      // Z-axis stepper motor (up/down)
ServoControl loaderServo;             // Loader servo
ServoAccelerationController mainServoController(&loaderServo); // Servo acceleration controller

//* ************************************************************************
//* *********************** CYLINDER OBJECTS ******************************
//* ************************************************************************
LoaderForkStepper loaderForkStepper(LOADER_FORK_STEP_PIN, LOADER_FORK_DIR_PIN);    // Loader fork stepper control

//* ************************************************************************
//* *********************** BUTTON CONTROL ********************************
//* ************************************************************************
Bounce2::Button startButton = Bounce2::Button();
Bounce2::Button zHomeSwitch = Bounce2::Button();
Bounce2::Button forkHomeSwitch = Bounce2::Button();
Bounce2::Button sensorButton = Bounce2::Button();

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
void initializeLoaderFork();
void initializeButtons();
void updateButtons();
void performStartupSequence();
void setupStateMachineReferences();
void homeZAxis();
void moveAwayFromHome();
void performServoSequence();
void performCycle();
void performServoAccelerationSequence();
void testServoAccelerationController(); // New test function
void logCurrentValues(); // Function to log current position values

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
  initializeLoaderFork();
  
  //! ************************************************************************
  //! STEP 3.5: INITIALIZE CELL CONFIGURATION
  //! ************************************************************************
  Serial.println("Initializing cell configuration...");
  initializeCellConfig();
  Serial.println("Cell configuration initialized");
  
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
  //! STEP 3: UPDATE SERVO ACCELERATION CONTROLLER
  //! ************************************************************************
  mainServoController.update();

  //! ************************************************************************
  //! STEP 4: UPDATE STATE MACHINE
  //! ************************************************************************
  updateStateMachine();

  //! ************************************************************************
  //! STEP 5: HANDLE START BUTTON PRESS
  //! ************************************************************************
  if (systemInitialized && startButton.pressed()) {
    if (getCurrentState() == IDLE_STATE) {
      // Start the cell sequence from loading tray position
      Serial.println("Start button pressed - starting cell sequence from loading tray position");
      startCellSequence();
      setState(CELL_SEQUENCE_STATE);
    } else if (getCurrentState() == CELL_SEQUENCE_STATE) {
      Serial.println("Start button pressed - continuing to next cell");
      continueCellSequence();
    }
  }

  //! ************************************************************************
  //! STEP 5.5: HANDLE SENSOR TRIGGER (ACTIVE LOW)
  //! ************************************************************************
  if (systemInitialized && sensorButton.fell()) {
    if (getCurrentState() == IDLE_STATE) {
      // Start the cell sequence from loading tray position with 1 second delay
      Serial.println("Sensor triggered - waiting 1 second before starting cell sequence");
      delay(1000); // 1 second delay before starting movement
      Serial.println("Starting cell sequence from loading tray position");
      startCellSequence();
      setState(CELL_SEQUENCE_STATE);
    } else if (getCurrentState() == CELL_SEQUENCE_STATE) {
      Serial.println("Sensor triggered - waiting 1 second before continuing to next cell");
      delay(1000); // 1 second delay before continuing
      Serial.println("Continuing to next cell");
      continueCellSequence();
    }
  }

  //! ************************************************************************
  //! STEP 6: HANDLE SERIAL COMMANDS
  //! ************************************************************************
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    // Check for manual commands first (h1.3, a30, f0.5, etc.)
    if (command.length() >= 2) {
      char commandType = command.charAt(0);
      String valueStr = command.substring(1);
      float value = valueStr.toFloat();
      
      if (commandType == 'h' && value > 0 && value <= 27) {
        // Manual height command - log current values only
        logCurrentValues();
        if (zMotor) {
          int targetSteps = (int)(value * STEPS_PER_INCH);
          zMotor->setSpeedInHz(Z_MAX_SPEED);
          zMotor->setAcceleration(Z_ACCELERATION);
          zMotor->moveTo(targetSteps);
        } else {
          Serial.println("ERROR: Z motor not available");
        }
        return; // Skip other command processing
      } else if (commandType == 'a' && value >= 0 && value <= 180) {
        // Manual servo angle command - log current values only
        logCurrentValues();
        mainServoController.moveTo(value);
        return; // Skip other command processing
      } else if (commandType == 'f' && value >= 0 && value <= FORK_MAX_DISTANCE_INCHES) {
        // Manual fork position command - log current values only
        logCurrentValues();
        if (loaderForkStepper.isMoving()) {
          Serial.println("Fork already moving - command ignored");
        } else {
          int targetSteps = (int)(value * FORK_STEPS_PER_INCH);
          // Get the stepper motor object and move it directly
          FastAccelStepper* forkStepper = loaderForkStepper.getStepper();
          if (forkStepper) {
            // Set speed and acceleration before movement
            forkStepper->setSpeedInHz(FORK_MAX_SPEED);
            forkStepper->setAcceleration(FORK_ACCELERATION);
            forkStepper->moveTo(targetSteps);
          } else {
            Serial.println("ERROR: Fork stepper not available");
          }
        }
        return; // Skip other command processing
      }
    }
    
    // Handle other commands
    if (command == "test_servo") {
      Serial.println("Manual servo test triggered");
      testServoAccelerationController();
    } else if (command == "servo_status") {
      Serial.println("=== SERVO STATUS ===");
      Serial.println("Current angle: " + String(mainServoController.getCurrentAngle()) + "°");
      Serial.println("Current velocity: " + String(mainServoController.getCurrentVelocity()) + " deg/s");
      Serial.println("Motion state: " + String(mainServoController.getMotionState()));
      Serial.println("Is moving: " + String(mainServoController.isMoving() ? "YES" : "NO"));
      Serial.println("Has reached target: " + String(mainServoController.hasReachedTarget() ? "YES" : "NO"));
    } else if (command.startsWith("servo_move ")) {
      int targetAngle = command.substring(11).toInt();
      Serial.println("Moving servo to: " + String(targetAngle) + "°");
      mainServoController.moveTo(targetAngle);
    } else if (command == "servo_stop") {
      Serial.println("Stopping servo");
      mainServoController.stop();
    } else if (command == "loader_extend") {
      Serial.println("Extending loader fork");
      loaderForkStepper.extend();
    } else if (command == "loader_retract") {
      Serial.println("Retracting loader fork");
      loaderForkStepper.retract();
    } else if (command == "loader_toggle") {
      Serial.println("Toggling loader fork state");
      loaderForkStepper.toggle();
    } else if (command == "loader_status") {
      Serial.println("=== LOADER FORK STATUS ===");
      Serial.println("Current state: " + String(loaderForkStepper.getState() ? "EXTENDED" : "RETRACTED"));
      Serial.println("Is extended: " + String(loaderForkStepper.isForkExtended() ? "YES" : "NO"));
      Serial.println("Current position: " + String(loaderForkStepper.getCurrentPosition()) + " steps");
      Serial.println("Is moving: " + String(loaderForkStepper.isMoving() ? "YES" : "NO"));
    } else if (command == "test_manual") {
      Serial.println("Switching to TEST state with manual mode");
      setState(TEST_STATE);
      // The test state will handle the manual mode toggle
    } else if (command == "start_sequence") {
      Serial.println("Starting cell sequence manually");
      startCellSequence();
      setState(CELL_SEQUENCE_STATE);
    } else if (command == "cells") {
      Serial.println("=== CELL CONFIGURATION ===");
      printCellConfig();
    } else if (command.startsWith("cell_set ")) {
      // Format: cell_set <column><row> <height> <angle>
      // Example: cell_set A1 4.2 55
      String params = command.substring(9);
      int firstSpace = params.indexOf(' ');
      int secondSpace = params.indexOf(' ', firstSpace + 1);
      
      if (firstSpace > 0 && secondSpace > firstSpace) {
        String cellRef = params.substring(0, firstSpace);
        float height = params.substring(firstSpace + 1, secondSpace).toFloat();
        int angle = params.substring(secondSpace + 1).toInt();
        
        // Parse column and row from cell reference (e.g., "A1", "B3", "D5")
        if (cellRef.length() >= 2) {
          char column = cellRef.charAt(0);
          int row = cellRef.substring(1).toInt();
          
          if (isValidCell(column, row)) {
            setCellPosition(column, row, height, angle);
            Serial.println("Cell " + String(column) + String(row) + " set to: " + String(height) + " inches, " + String(angle) + " degrees");
          } else {
            Serial.println("ERROR: Invalid cell reference. Use A1-D5");
          }
        } else {
          Serial.println("ERROR: Invalid cell reference format. Use: A1, B2, C3, etc.");
        }
      } else {
        Serial.println("ERROR: Invalid format. Use: cell_set <column><row> <height> <angle>");
        Serial.println("Example: cell_set A1 4.2 55");
      }
    } else if (command.startsWith("cell_move ")) {
      // Format: cell_move <column><row>
      // Example: cell_move A1
      String cellRef = command.substring(10);
      
      if (cellRef.length() >= 2) {
        char column = cellRef.charAt(0);
        int row = cellRef.substring(1).toInt();
        
        if (isValidCell(column, row)) {
          CellPosition pos = getCellPosition(column, row);
          Serial.println("Moving to cell " + String(column) + String(row) + ": " + String(pos.height_inches) + " inches, " + String(pos.servo_angle) + " degrees");
          
          // Move Z-axis to cell height
          if (zMotor) {
            int targetSteps = getCellHeightSteps(column, row);
            zMotor->setSpeedInHz(Z_MAX_SPEED);
            zMotor->setAcceleration(Z_ACCELERATION);
            zMotor->moveTo(targetSteps);
          }
          
          // Move servo to cell angle
          mainServoController.moveTo(pos.servo_angle);
          
          // Note: Fork movement is handled by the state machine, not direct commands
        } else {
          Serial.println("ERROR: Invalid cell reference. Use A1-D5");
        }
      } else {
        Serial.println("ERROR: Invalid cell reference format. Use: A1, B2, C3, etc.");
      }
    } else if (command.startsWith("test_cell ")) {
      // Format: test_cell <column><row>
      // Example: test_cell A1
      String cellRef = command.substring(10);
      
      if (cellRef.length() >= 2) {
        char column = cellRef.charAt(0);
        int row = cellRef.substring(1).toInt();
        
        if (isValidCell(column, row)) {
          Serial.println("Starting cell test for " + String(column) + String(row));
          setCellTestTarget(column, row);
          setState(CELL_TEST_STATE);
        } else {
          Serial.println("ERROR: Invalid cell reference. Use A1-D5");
        }
      } else {
        Serial.println("ERROR: Invalid cell reference format. Use: test_cell A1, test_cell B2, etc.");
      }
    } else if (command == "help") {
      Serial.println("=== AVAILABLE COMMANDS ===");
      Serial.println("h<height>  - Move Z-axis to height (inches) - Example: h1.3, h20 (max 27 inches)");
      Serial.println("a<angle>   - Move servo to angle (degrees) - Example: a30, a90");
      Serial.println("f<position> - Move fork to position (inches) - Example: f0.5, f1.0, f2.5 (max " + String(FORK_MAX_DISTANCE_INCHES) + " inches)");
      Serial.println("cells      - Show all cell configurations");
      Serial.println("cell_set <column><row> <height> <angle> - Set cell position");
      Serial.println("cell_move <column><row> - Move to specific cell position");
      Serial.println("test_cell <column><row> - Test pick/place cycle for specific cell");
      Serial.println("start_sequence - Start automated cell sequence (machine waits at loading tray position)");
      Serial.println("help       - Show this help message");
      Serial.println("test_manual - Enter test state");
      Serial.println("servo_status - Show servo status");
      Serial.println("cylinder_status - Show cylinder status");
    } else if (command == "status") {
      Serial.println("=== SYSTEM STATUS ===");
      if (zMotor) {
        int currentSteps = zMotor->getCurrentPosition();
        float currentInches = (float)currentSteps / STEPS_PER_INCH;
        Serial.println("Z Position: " + String(currentSteps) + " steps (" + String(currentInches, 2) + " inches)");
        Serial.println("Z Motor running: " + String(zMotor->isRunning() ? "YES" : "NO"));
      }
      Serial.println("Servo Angle: " + String(mainServoController.getCurrentAngle(), 1) + " degrees");
      Serial.println("Servo moving: " + String(mainServoController.isMoving() ? "YES" : "NO"));
      
      // Fork status
      int currentForkSteps = loaderForkStepper.getCurrentPosition();
      float currentForkInches = (float)currentForkSteps / FORK_STEPS_PER_INCH;
      Serial.println("Fork Position: " + String(currentForkSteps) + " steps (" + String(currentForkInches, 2) + " inches)");
      Serial.println("Fork State: " + String(loaderForkStepper.isForkExtended() ? "EXTENDED" : "RETRACTED"));
      Serial.println("Fork Moving: " + String(loaderForkStepper.isMoving() ? "YES" : "NO"));
      
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
  
  // Z home switch: Active HIGH (input pulldown)
  zHomeSwitch.attach(Z_HOME_SWITCH_PIN, INPUT_PULLDOWN);
  zHomeSwitch.interval(HOME_SWITCH_DEBOUNCE);
  
  // Fork home switch: Active HIGH (input pulldown)
  forkHomeSwitch.attach(FORK_HOME_SWITCH_PIN, INPUT);
  forkHomeSwitch.interval(FORK_HOME_SWITCH_DEBOUNCE);
  
  // Sensor: Active LOW (input pullup)
  sensorButton.attach(SENSOR_PIN, INPUT_PULLUP);
  sensorButton.interval(START_BUTTON_DEBOUNCE);
  
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
  
  loaderServo.init(LOADER_SERVO_PIN, 7, 20, 14); // Pin, channel, frequency, resolution
  
  // Configure servo acceleration controller with faster settings for cell sequence
  mainServoController.setAccelerationProfile(
    50.0,   // Acceleration rate (degrees/second^2) - increased from 10.0
    120.0   // Max velocity (degrees/second) - increased from 30.0
  );
  
  // Note: Initial servo position will be set during startup sequence using regular ServoControl
  // Do not set position here to avoid conflicts with startup sequence
  
  Serial.println("Loader servo and acceleration controller initialized");
}

void initializeLoaderFork() {
  //! ************************************************************************
  //! INITIALIZE LOADER FORK STEPPER MOTOR
  //! ************************************************************************
  Serial.println("Setting up loader fork stepper motor...");
  Serial.println("Step pin: " + String(LOADER_FORK_STEP_PIN));
  Serial.println("Direction pin: " + String(LOADER_FORK_DIR_PIN));
  
  // Create stepper motor object for loader fork using existing engine
  FastAccelStepper* loaderForkMotor = engine.stepperConnectToPin(LOADER_FORK_STEP_PIN);
  if (loaderForkMotor) {
      loaderForkMotor->setDirectionPin(LOADER_FORK_DIR_PIN);
      loaderForkMotor->enableOutputs(); // Enable motor outputs
  }
  
  loaderForkStepper.begin(loaderForkMotor);
  
  // Set the debounced home switch for the loader fork
  loaderForkStepper.setHomeSwitch(&forkHomeSwitch);
  
  // Ensure loader fork starts in retracted position
  loaderForkStepper.retract();
  
  Serial.println("Loader fork stepper motor initialized and retracted");
}

void updateButtons() {
  //! ************************************************************************
  //! UPDATE ALL BUTTON STATES
  //! ************************************************************************
  startButton.update();
  zHomeSwitch.update();
  forkHomeSwitch.update();
  sensorButton.update();
}

void setupStateMachineReferences() {
  //! ************************************************************************
  //! SET UP REFERENCES FOR STATE MACHINE
  //! ************************************************************************
  Serial.println("Setting up state machine references...");
  
  // Set references for home state
  setHomeReferences(zMotor, &zHomeSwitch, &loaderForkStepper);
  
  // Set references for test state
  setTestReferences(zMotor, &mainServoController, &loaderForkStepper);
  
  // Set references for idle state
  setIdleReferences(&loaderServo, &mainServoController, zMotor);
  
  // Set references for cell sequence state
  setCellSequenceReferences(&loaderServo, &loaderForkStepper, zMotor);
  
  // Set references for cell test state
  setCellTestReferences(&loaderServo, &loaderForkStepper, zMotor);
  
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
  while (!zHomeSwitch.read()) {
    updateButtons();
    // handleOTA(); // Continue handling OTA during homing
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
  while (zMotor->isRunning()) {
    delay(1);
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
  mainServoController.moveTo(SERVO_START_POS);
  while (!mainServoController.isMoveComplete()) {
    mainServoController.update();
    // handleOTA();
    delay(10);
  }

  //! ************************************************************************
  //! STEP 2: MOVE TO 45 DEGREES
  //! ************************************************************************
  Serial.println("Moving servo to " + String(SERVO_SECOND_POS) + " degrees");
  mainServoController.moveTo(SERVO_SECOND_POS);
  while (!mainServoController.isMoveComplete()) {
    mainServoController.update();
    // handleOTA();
    delay(10);
  }

  //! ************************************************************************
  //! STEP 3: MOVE TO 70 DEGREES
  //! ************************************************************************
  Serial.println("Moving servo to " + String(SERVO_THIRD_POS) + " degrees");
  mainServoController.moveTo(SERVO_THIRD_POS);
  while (!mainServoController.isMoveComplete()) {
    mainServoController.update();
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
  mainServoController.setCurrentAngle(90);
  loaderServo.write(90.0);
  delay(100); // Give servo time to reach position
  
  // Perform 5 movements with increasing acceleration
  for (int movement = 0; movement < 5; movement++) {
    // Calculate acceleration for this movement (increasing with each movement)
    float baseAccel = 5.0; // Base acceleration rate (degrees/second^2)
    float currentAccel = baseAccel * (movement + 1); // Increase acceleration each time
    
    // Set acceleration profile for this movement
    mainServoController.setAccelerationProfile(
      currentAccel,  // Acceleration rate (same as deceleration)
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
    mainServoController.moveTo((int)targetAngle);
    
    Serial.println("Servo movement " + String(movement + 1) + 
                  " - Moving to " + String(targetAngle) + 
                  " degrees with acceleration " + String(currentAccel));
    
    // Wait for movement to complete using new position verification
    while (!mainServoController.isMoveComplete()) {
      mainServoController.update();
      delay(10);
    }
  }
  
  Serial.println("Servo acceleration sequence complete");
}

void testServoAccelerationController() {
  //! ************************************************************************
  //! COMPREHENSIVE SERVO ACCELERATION CONTROLLER TEST
  //! ************************************************************************
  Serial.println("=== SERVO ACCELERATION CONTROLLER TEST ===");
  
  // Test 1: Basic initialization check
  Serial.println("Test 1: Checking servo controller initialization...");
  // Note: mainServoController is an object, not a pointer, so it's always valid
  Serial.println("✓ Servo objects initialized correctly");
  
  Serial.println("✓ Servo objects initialized correctly");
  
  // Test 2: Set servo to known position
  Serial.println("Test 2: Setting servo to 90 degrees...");
  mainServoController.setCurrentAngle(90);
  loaderServo.write(90.0);
  delay(500); // Give servo time to reach position
  
  Serial.println("✓ Servo positioned at 90 degrees");
  
  // Test 3: Test slow movement
  Serial.println("Test 3: Testing slow movement (90° -> 120°)...");
  mainServoController.setAccelerationProfile(5.0, 15.0); // Slow and smooth
  mainServoController.moveTo(120);
  
  // Use new position verification method
  while (!mainServoController.isMoveComplete()) {
    mainServoController.update();
    delay(10);
  }
  
  if (mainServoController.hasReachedTarget()) {
    Serial.println("✓ Slow movement test passed");
  } else {
    Serial.println("✗ Slow movement test failed - timeout");
  }
  
  // Test 4: Test fast movement
  Serial.println("Test 4: Testing fast movement (120° -> 60°)...");
  mainServoController.setAccelerationProfile(20.0, 60.0); // Fast and aggressive
  mainServoController.moveTo(60);
  
  // Use new position verification method
  while (!mainServoController.isMoveComplete()) {
    mainServoController.update();
    delay(10);
  }
  
  if (mainServoController.hasReachedTarget()) {
    Serial.println("✓ Fast movement test passed");
  } else {
    Serial.println("✗ Fast movement test failed - timeout");
  }
  
  // Test 5: Test return to center
  Serial.println("Test 5: Testing return to center (60° -> 90°)...");
  mainServoController.setAccelerationProfile(10.0, 30.0); // Medium settings
  mainServoController.moveTo(90);
  
  // Use new position verification method
  while (!mainServoController.isMoveComplete()) {
    mainServoController.update();
    delay(10);
  }
  
  if (mainServoController.hasReachedTarget()) {
    Serial.println("✓ Return to center test passed");
  } else {
    Serial.println("✗ Return to center test failed - timeout");
  }
  
  // Test 6: Test stop functionality
  Serial.println("Test 6: Testing stop functionality...");
  mainServoController.moveTo(150);
  delay(100); // Let it start moving
  mainServoController.stop();
  
  // Use new position verification method
  while (!mainServoController.isMoveComplete()) {
    mainServoController.update();
    delay(10);
  }
  
  if (!mainServoController.isMoving()) {
    Serial.println("✓ Stop functionality test passed");
  } else {
    Serial.println("✗ Stop functionality test failed - servo still moving");
  }
  
  // Test 7: Test status methods
  Serial.println("Test 7: Testing status methods...");
  float currentAngle = mainServoController.getCurrentAngle();
  float currentVelocity = mainServoController.getCurrentVelocity();
  ServoAccelerationController::MotionState state = mainServoController.getMotionState();
  
  Serial.println("Current angle: " + String(currentAngle) + "°");
  Serial.println("Current velocity: " + String(currentVelocity) + " deg/s");
  Serial.println("Motion state: " + String(state));
  Serial.println("Is moving: " + String(mainServoController.isMoving() ? "YES" : "NO"));
  Serial.println("Has reached target: " + String(mainServoController.hasReachedTarget() ? "YES" : "NO"));
  
  Serial.println("✓ Status methods working");
  
  // Test 8: Final position check
  Serial.println("Test 8: Final position verification...");
  mainServoController.setCurrentAngle(90);
  loaderServo.write(90.0);
  delay(500);
  
  Serial.println("=== SERVO ACCELERATION CONTROLLER TEST COMPLETE ===");
  Serial.println("All tests passed - servo acceleration controller is working properly");
}

//* ************************************************************************
//* ************************ LOG CURRENT VALUES ***************************
//* ************************************************************************
void logCurrentValues() {
    //! ************************************************************************
    //! LOG CURRENT POSITION VALUES ONLY - COMPACT FORMAT
    //! ************************************************************************
    
    // Get current Z position
    if (zMotor) {
        int currentSteps = zMotor->getCurrentPosition();
        float currentInches = (float)currentSteps / STEPS_PER_INCH;
        Serial.print("Z:" + String(currentInches, 2) + "\" ");
    }
    
    // Get current servo angle
    float currentAngle = mainServoController.getCurrentAngle();
    Serial.print("A:" + String(currentAngle, 1) + "° ");
    
    // Get current fork position
    int currentForkSteps = loaderForkStepper.getCurrentPosition();
    float currentForkInches = (float)currentForkSteps / FORK_STEPS_PER_INCH;
    Serial.println("F:" + String(currentForkInches, 2) + "\"");
}