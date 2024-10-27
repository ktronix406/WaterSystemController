// Relay control pins
const int deepWellPumpPin = 12;
const int pondFillValvePin = 11;
const int spigotValvePin = 8;

// Remaining digital pins for non-relay controls
const int floatValve1MinPin = 2;
const int floatValve2MaxPin = 3;
const int interruptButtonPin = 4;
const int buttonIndicator = 5;

// Analog pins used as digital for additional controls
const int winterModeButtonPin = A0;
const int deactivateWinterModeButtonPin = A1;
const int ledWinterModeActivePin = A2;
const int ledWinterModeInactivePin = A3;

const int pauseButtonPin = A4;
const int ledPauseActivePin = A5;

unsigned long lastFloatCheckTime = 0;
const unsigned long floatCheckInterval = 2 * 60 * 1000UL;  // 2 minutes in milliseconds

int floatMinState = 0;
int floatMaxState = 0;
bool isPumpOn = false;
bool pondFillStage = false;
bool isWinterMode = false;
bool cisternFill = false;
bool autoPondFillActive = false;
bool isInAutoFillMode = false;
bool isPaused = false;
bool buttonPressed = false;  // New global variable

const unsigned long debounceDelay = 500;  // Set to 500 milliseconds; adjust as needed

void setup() {
  pinMode(deepWellPumpPin, OUTPUT);
  pinMode(pondFillValvePin, OUTPUT);
  pinMode(spigotValvePin, OUTPUT);
  pinMode(floatValve1MinPin, INPUT_PULLUP);
  pinMode(floatValve2MaxPin, INPUT_PULLUP);
  pinMode(interruptButtonPin, INPUT_PULLUP);
  pinMode(buttonIndicator, OUTPUT);
  pinMode(winterModeButtonPin, INPUT_PULLUP);
  pinMode(deactivateWinterModeButtonPin, INPUT_PULLUP);
  pinMode(ledWinterModeActivePin, OUTPUT);
  pinMode(ledWinterModeInactivePin, OUTPUT);
  pinMode(pauseButtonPin, INPUT_PULLUP);  // Set the pause button as input with pull-up
  pinMode(ledPauseActivePin, OUTPUT);     // Set the pause LED as output

  // Initialize the state of LEDs (assuming winter mode is inactive at start)
  digitalWrite(ledWinterModeActivePin, LOW);
  digitalWrite(ledWinterModeInactivePin, HIGH);
  digitalWrite(ledPauseActivePin, LOW);  // Initialize LED to be OFF

  Serial.begin(9600);

  resetToDefault();
  Serial.println("System initialized or a problem occurred.");
}

void loop() {
  checkPauseButton();
  checkSerialCommands();

  if (isPaused) {
    return;  // Exit the loop function, effectively pausing all activity
  }

  checkWinterModeButtons();
  checkButtonPress();

  unsigned long currentTime = millis();
  if (currentTime - lastFloatCheckTime >= floatCheckInterval) {
    handleFloatSwitches();
    lastFloatCheckTime = currentTime;
  }

  if (autoPondFillActive) {
    controlDeepWellPump(true);
  }
}

void processCommand(String command) {
  command.toLowerCase();
  if (isPaused || command == "") {
    Serial.println("System is paused. Command ignored.");
    return;
  }

  if (isWinterMode && (command == "pond fill on" || command == "pond fill reset")) {
    Serial.println("Winter mode active. Auto-fill interrupted.");
    return;
  }

  if (command == "pump on") {
    autoPondFillActive = false;
    controlDeepWellPump(true);
    Serial.println("Pump turned on.");
  } else if (command == "pump off") {
    isPumpOn = false;
    autoPondFillActive = false;
    controlDeepWellPump(false);
    controlPondFillValve(false);
    controlSpigotValve(false);
    pondFillStage = false;
    Serial.println("Pump turned off.");
  } else if (command == "pond fill on" && !cisternFill) {
    autoPondFillActive = true;
    controlDeepWellPump(true);
    controlPondFillValve(true);
    controlSpigotValve(false);
    Serial.println("Pond fill initiated.");
    handleFloatSwitches();
  } else if (command == "pond fill off") {
    pondFillStage = false;
    autoPondFillActive = false;
    controlDeepWellPump(false);
    controlPondFillValve(false);
    controlSpigotValve(false);
    Serial.println("Pond fill interrupted.");
  } else if (command == "pond fill reset") {
    pondFillStage = false;
    autoPondFillActive = true;
    controlPondFillValve(false);
    controlSpigotValve(false);
    Serial.println("Pond fill reset.");
    handleFloatSwitches();
  } else if (command == "open fill valve" && !cisternFill) {
    controlPondFillValve(true);
    Serial.println("Pond fill valve opened.");
  } else if (command == "close fill valve") {
    controlPondFillValve(false);
    Serial.println("Pond fill valve closed.");
  } else if (command == "open spigot valve") {
    controlSpigotValve(true);
    Serial.println("Spigot valve opened.");
  } else if (command == "close spigot valve") {
    controlSpigotValve(false);
    Serial.println("Spigot valve closed.");
  } else if (command == "check float status") {
    checkFloatStatus();
  } else if (command == "activate winter mode") {
    isWinterMode = true;
    resetToDefault();
    Serial.println("Winter mode activated. All systems reset to default.");
    digitalWrite(ledWinterModeActivePin, HIGH);
    digitalWrite(ledWinterModeInactivePin, LOW);
  } else if (command == "deactivate winter mode") {
    isWinterMode = false;
    autoPondFillActive = true;
    Serial.println("Winter mode deactivated.");
    handleFloatSwitches();
    digitalWrite(ledWinterModeActivePin, LOW);
    digitalWrite(ledWinterModeInactivePin, HIGH);
  } else if (command == "activate cistern fill") {
    activateCisternFill();
  } else if (command == "deactivate cistern fill") {
    deactivateCisternFill();
  } else {
    Serial.println("Unknown command: " + command);
  }
}

void checkButtonPress() {
  static bool lastButtonState = HIGH;
  static unsigned long lastDebounceTime = 0;
  static int savedFloatMinState = LOW;
  static int savedFloatMaxState = LOW;

  bool currentButtonState = digitalRead(interruptButtonPin);

  if (currentButtonState != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (!buttonPressed && currentButtonState == LOW) {
      buttonPressed = true;
      pondFillStage = false;
      autoPondFillActive = false;
      controlDeepWellPump(true);
      controlPondFillValve(false);
      controlSpigotValve(true);
      digitalWrite(buttonIndicator, HIGH);
      savedFloatMinState = digitalRead(floatValve1MinPin);
      savedFloatMaxState = digitalRead(floatValve2MaxPin);
      Serial.println("Button pressed: Pond fill interrupted, spigot opened.");
    } else if (buttonPressed && currentButtonState == HIGH) {
      buttonPressed = false;
      controlDeepWellPump(false);
      controlSpigotValve(false);
      int currentFloatMinState = digitalRead(floatValve1MinPin);
      int currentFloatMaxState = digitalRead(floatValve2MaxPin);
      if (currentFloatMinState == savedFloatMinState && currentFloatMaxState == savedFloatMaxState) {
        autoPondFillActive = true;
        controlPondFillValve(true);
        Serial.println("Button released: Resuming pond fill.");
      } else {
        Serial.println("Button released: Float states changed, not resuming pond fill.");
      }
      digitalWrite(buttonIndicator, LOW);
    }
  }

  lastButtonState = currentButtonState;
}

void checkFloatStatus() {
  floatMinState = digitalRead(floatValve1MinPin);
  floatMaxState = digitalRead(floatValve2MaxPin);
  Serial.print(floatMinState);
  Serial.print(" - ");
  Serial.print(floatMaxState);
  Serial.println(" Float Status");
}

void activateCisternFill() {
  cisternFill = true;
  pondFillStage = false;
  controlPondFillValve(false);
  controlSpigotValve(true);
  Serial.println("Cistern fill activated.");
}

void deactivateCisternFill() {
  cisternFill = false;
  controlSpigotValve(false);
  Serial.println("Cistern fill deactivated.");
}

void handleFloatSwitches() {
  static int prevFloatMinState = -1;
  static int prevFloatMaxState = -1;

  int floatMinState = digitalRead(floatValve1MinPin);
  int floatMaxState = digitalRead(floatValve2MaxPin);

  if (!isWinterMode) {
    if (floatMinState == LOW && floatMaxState == HIGH && (prevFloatMinState != floatMinState || prevFloatMaxState != floatMaxState)) {
      isInAutoFillMode = true;
      autoPondFillActive = true;
      controlDeepWellPump(true);
      controlPondFillValve(true);
      controlSpigotValve(false);
      Serial.println("Low water level detected. Auto-filling pond.");
    } else if (floatMinState == LOW && floatMaxState == LOW && (prevFloatMinState != floatMinState || prevFloatMaxState != floatMaxState)) {
      isInAutoFillMode = true;
      autoPondFillActive = true;
      controlDeepWellPump(true);
      controlPondFillValve(true);
      controlSpigotValve(false);
      Serial.println("Low water level detected. Auto-filling pond.");
    } else if (floatMinState == HIGH && floatMaxState == HIGH && (prevFloatMinState != floatMinState || prevFloatMaxState != floatMaxState)) {
      isInAutoFillMode = false;
      autoPondFillActive = false;
      controlDeepWellPump(false);
      controlPondFillValve(false);
      controlSpigotValve(false);
      Serial.println("High water level detected. Auto-fill stopped.");
      // Ensure spigot valve reopens if it was previously opened by the button
      if (buttonPressed) {
        controlSpigotValve(true);
        Serial.println("High water level detected: Spigot reopened.");
      }
    } else if (prevFloatMinState != floatMinState || prevFloatMaxState != floatMaxState) {
      autoPondFillActive = false;
      Serial.println("Error: Invalid sensor readings.");
    }
  } else {
    if (prevFloatMinState != floatMinState || prevFloatMaxState != floatMaxState) {
      autoPondFillActive = false;
      controlPondFillValve(false);
      controlSpigotValve(false);
      Serial.println("Winter mode active. Auto pond fill disabled.");
    }
  }

  prevFloatMinState = floatMinState;
  prevFloatMaxState = floatMaxState;
}

void controlDeepWellPump(bool on) {
  digitalWrite(deepWellPumpPin, on ? LOW : HIGH);
}

void controlPondFillValve(bool open) {
  digitalWrite(pondFillValvePin, open ? LOW : HIGH);
}

void controlSpigotValve(bool open) {
  digitalWrite(spigotValvePin, open ? LOW : HIGH);
}

void resetToDefault() {
  controlDeepWellPump(false);
  controlPondFillValve(false);
  controlSpigotValve(false);
  pondFillStage = false;
  cisternFill = false;
  autoPondFillActive = false;
  isInAutoFillMode = false;
  buttonPressed = false;  // Reset button state
  Serial.println("All systems reset to default.");
}

void checkSerialCommands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    processCommand(command);
  }
}

void checkWinterModeButtons() {
  static bool lastWinterButtonState = HIGH;
  static bool lastDeactivateWinterButtonState = HIGH;
  static bool debounceWinter = false;
  static bool debounceDeactivateWinter = false;
  static unsigned long lastDebounceTimeWinter = 0;
  static unsigned long lastDebounceTimeDeactivateWinter = 0;

  bool currentWinterButtonState = digitalRead(winterModeButtonPin);
  bool currentDeactivateWinterButtonState = digitalRead(deactivateWinterModeButtonPin);

  // Debounce winter mode activation button
  if (currentWinterButtonState != lastWinterButtonState) {
    lastDebounceTimeWinter = millis();
  }
  if ((millis() - lastDebounceTimeWinter) > debounceDelay && !debounceWinter) {
    if (currentWinterButtonState == LOW) {
      isWinterMode = true;
      debounceWinter = true;
      digitalWrite(ledWinterModeActivePin, HIGH);
      digitalWrite(ledWinterModeInactivePin, LOW);
      Serial.println("Winter mode activated.");
    }
  } else if (currentWinterButtonState == HIGH) {
    debounceWinter = false;
  }

  // Debounce winter mode deactivation button
  if (currentDeactivateWinterButtonState != lastDeactivateWinterButtonState) {
    lastDebounceTimeDeactivateWinter = millis();
  }
  if ((millis() - lastDebounceTimeDeactivateWinter) > debounceDelay && !debounceDeactivateWinter) {
    if (currentDeactivateWinterButtonState == LOW) {
      isWinterMode = false;
      debounceDeactivateWinter = true;
      digitalWrite(ledWinterModeActivePin, LOW);
      digitalWrite(ledWinterModeInactivePin, HIGH);
      Serial.println("Winter mode deactivated.");
    }
  } else if (currentDeactivateWinterButtonState == HIGH) {
    debounceDeactivateWinter = false;
  }

  lastWinterButtonState = currentWinterButtonState;
  lastDeactivateWinterButtonState = currentDeactivateWinterButtonState;
}

void checkPauseButton() {
  static bool lastPauseButtonState = HIGH;
  static bool debouncePause = false;
  static unsigned long lastDebounceTimePause = 0;

  bool currentPauseButtonState = digitalRead(pauseButtonPin);

  if (currentPauseButtonState != lastPauseButtonState) {
    lastDebounceTimePause = millis();
  }

  if ((millis() - lastDebounceTimePause) > debounceDelay) {
    if (!debouncePause && currentPauseButtonState == LOW) {
      isPaused = !isPaused;
      debouncePause = true;

      if (isPaused) {
        digitalWrite(ledWinterModeActivePin, LOW);
        digitalWrite(ledWinterModeInactivePin, LOW);
        digitalWrite(buttonIndicator, LOW);
        digitalWrite(ledPauseActivePin, HIGH);
        Serial.println("System paused.");
        pauseAllOperations();
      } else {
        digitalWrite(ledWinterModeActivePin, HIGH);
        digitalWrite(ledWinterModeInactivePin, HIGH);
        digitalWrite(ledPauseActivePin, LOW);
        Serial.println("System unpaused.");
        resumeAllOperations();
      }
    } else if (currentPauseButtonState == HIGH) {
      debouncePause = false;
    }
  }

  lastPauseButtonState = currentPauseButtonState;
}

void pauseAllOperations() {
  controlDeepWellPump(false);
  controlPondFillValve(false);
  controlSpigotValve(false);
}

void resumeAllOperations() {
  handleFloatSwitches();
  checkButtonPress();

  if (isWinterMode) {
    digitalWrite(ledWinterModeActivePin, HIGH);
    digitalWrite(ledWinterModeInactivePin, LOW);
  } else {
    digitalWrite(ledWinterModeActivePin, LOW);
    digitalWrite(ledWinterModeInactivePin, HIGH);
  }
}
