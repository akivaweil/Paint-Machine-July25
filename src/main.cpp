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
// Z-axis stepper motor (up/down) with X-axis servo rotation
// OTA functionality integrated for remote updates

// External OTA functions
extern void setupOTA();
extern void handleOTA();

//* ************************************************************************
//* *********************** MOTOR & SERVO OBJECTS *************************
//* ************************************************************************
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *zMotor = NULL;      // Z-axis stepper motor (up/down)
ServoControl xServo;                  // X-axis rotation servo

//* ************************************************************************
//* *********************** BUTTON CONTROL ********************************
//* ************************************************************************
Bounce2::Button startButton = Bounce2::Button();
Bounce2::Button stopButton = Bounce2::Button();
Bounce2::Button zHomeSwitch = Bounce2::Button();
Bounce2::Button zBottomSwitch = Bounce2::Button();

//* ************************************************************************
//* *********************** FUNCTION DECLARATIONS *************************
//* ************************************************************************
void initializeMotor();
void initializeServo();
void initializeButtons();
void updateButtons();

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
  //! STEP 3: INITIALIZE LOADER SYSTEMS
  //! ************************************************************************
  Serial.println("Initializing loader systems...");
  
  initializeButtons();
  initializeMotor();
  initializeServo();
  
  Serial.println("Loader systems initialized");
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
  //! STEP 3: LOADER MAIN LOOP
  //! ************************************************************************
  // Check for button presses and handle loader operations
  if (startButton.pressed()) {
    Serial.println("Start button pressed - ready for loader cycle implementation");
  }
  
  if (stopButton.pressed()) {
    Serial.println("Stop button pressed - emergency stop");
    if (zMotor) {
      zMotor->forceStop();
    }
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
  //! INITIALIZE BUTTONS WITH PROPER INPUT MODES
  //! ************************************************************************
  Serial.println("Setting up buttons and switches...");
  
  // Physical switches: Active HIGH (input pulldown)
  startButton.attach(START_BUTTON_PIN, INPUT_PULLDOWN);
  startButton.interval(50);
  
  stopButton.attach(STOP_BUTTON_PIN, INPUT_PULLDOWN);
  stopButton.interval(50);
  
  // Sensors: Active LOW (input pullup)
  zHomeSwitch.attach(Z_HOME_SWITCH_PIN, INPUT_PULLUP);
  zHomeSwitch.interval(50);
  
  zBottomSwitch.attach(Z_BOTTOM_SWITCH_PIN, INPUT_PULLUP);
  zBottomSwitch.interval(50);
  
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
    zMotor->setAutoEnable(false); // Manual enable control
    zMotor->setCurrentPosition(0);
    
    // Setup enable pin
    pinMode(Z_MOTOR_ENABLE_PIN, OUTPUT);
    digitalWrite(Z_MOTOR_ENABLE_PIN, LOW); // Enable motor (active low)
    
    Serial.println("Z-axis motor configured successfully");
    Serial.println("Z Motor speed: " + String(Z_MAX_SPEED) + " Hz");
    Serial.println("Z Motor acceleration: " + String(Z_ACCELERATION) + " steps/s²");
  } else {
    Serial.println("ERROR: Failed to create Z-axis motor instance");
  }
}

void initializeServo() {
  //! ************************************************************************
  //! INITIALIZE X-AXIS SERVO FOR ROTATION CONTROL
  //! ************************************************************************
  Serial.println("Setting up X-axis rotation servo...");
  
  xServo.init(X_SERVO_PIN, 0, 50, 16); // Pin, channel, frequency, resolution
  xServo.write(SERVO_NEUTRAL_POS);     // Start in neutral position
  
  Serial.println("X-axis servo initialized at neutral position: " + String(SERVO_NEUTRAL_POS) + "°");
}

void updateButtons() {
  //! ************************************************************************
  //! UPDATE ALL BUTTON STATES
  //! ************************************************************************
  startButton.update();
  stopButton.update();
  zHomeSwitch.update();
  zBottomSwitch.update();
}