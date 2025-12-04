/*
This code contains three main control elements: 
buttons (start/stop), solenoid valve activation, reading of ultrasonic sensor.

Created: 02/04/2025
by Zenllaze Gomez Merlo
*/

/// Setup pins for buttons start, stop, and mock level sensor
const byte bStart = 11; 
const byte bStop = 12; 
const byte solValve = 13;

// define pins for ultrasonic sensor
const byte PIN_trig =  9;  
const byte PIN_echo =  10; 
float distance = 0; 
 

// Define max of funnel
float MAX_FUNNELHEIGHT = 5;  // NOT ACTUAL NUMBER; controls turn off of pump

// ultrasonic reading for whether valve needs to be open or closed
bool uValve = true;

/*
Function to activate solenoid
*/
void activateSolenoid() {
  digitalWrite(solValve, LOW); 
  Serial.print("Solenoid activated");
  delay(200); 
}

/*
Function to deactivate solenoid`1m
*/
void deactivateSolenoid() {
  digitalWrite(solValve, HIGH); 
  Serial.print("Solenoid deactivated");
  delay(200);
}

/* 
Function for button debounce
  @input byte with the pin number and boolean with previous state
  @returns current state or does a digital read
*/
bool debounce(byte pin, bool prevState) {
  bool currState = digitalRead(pin);
  if (currState != prevState) {
    delay(5); // Debounce delay
    return digitalRead(pin);
  }
  return currState;
}

/*
  Function to convert centimeters to inches
    @input float storing distance in cm
    @return float with distance in inches
*/
float calcInches(float cm) {
  return cm / 2.54; // conversion
}

/*
 Function to send signal to the sensor and return distance
  @return float with the distance of the liquid
 */
 float SonarDistance(){
  unsigned long pulseDuration = 0;
  float distance_cm = 0;

  digitalWrite(PIN_trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_trig, LOW);
  pulseDuration = pulseIn(PIN_echo, HIGH);

  distance_cm = ((pulseDuration*.000001) * 34000)/2; //distance conversion to cm using speed of sound divided by two because wave travels the distance is twice

  return distance_cm;
 }

/*
 Function to read distance
  @return value of whether the valve needs to be turned off 
 */
bool LiquidDistance() {
  // Read distance from transceiver and convert to inches
  float distance = calcInches(SonarDistance());
  
  if (distance == 0) {
    uValve = true; // Close valve if out of range TO DO:range fix?
    return uValve;
  }
  
  Serial.print("Distance: ");
  Serial.print(distance); // Print distance in inches
  Serial.println(" inches");

  if (distance <= MAX_FUNNELHEIGHT) { // Measured distance of rising liquid less than max height of funnel
    Serial.println("Valve off, funnel height too high"); 
    uValve = true; // Close valve
  } else {
    Serial.println("Valve on, funnel height in range"); 
    uValve = false; // Open valve
  }

  return uValve; // Return valve state
}

/* 
Function run at initialization
*/
void setup() {
  pinMode(bStart, INPUT_PULLUP);  // Start button
  pinMode(bStop, INPUT_PULLUP);   // Stop button

  pinMode(solValve, OUTPUT);      // Solenoid valve pin as output
  digitalWrite(solValve, HIGH);    // Closed valve at the beginning

  pinMode(PIN_trig, OUTPUT); // Trigger for ultrasonic sensor
  pinMode(PIN_echo, INPUT); // Echo for ultrasonic sensor
  digitalWrite(PIN_trig, LOW); // No signal sent

  Serial.begin(9600);
}

/*
Function that microcontroller loops through
*/
void loop() {
  // Declare static variables for button states
  static bool stateStart = digitalRead(bStart);
  static bool stateStartPrior = stateStart;
  static bool stateStop = digitalRead(bStop);
  static bool stateStopPrior = stateStop;

  // Declare static variables for pump state
  static bool started = false;  // true is on, false is off
  static bool stopped = true;   // Start with pump stopped initially
  static bool stateUSensor = true;

  // Debounce and update button states
  stateStart = debounce(bStart, stateStart);
  stateStop = debounce(bStop, stateStop);

  // Update ultrasonic sensor distance state
  stateUSensor = LiquidDistance();   // Update state of liquid rising

  // Handle stop button pressed (turn off solenoid if pressed)
  if (stateStop == LOW && stateStopPrior == HIGH) {
    deactivateSolenoid();
    Serial.println("Pump stopped by stop button");
    started = false;  // Pump is stopped
    stopped = true;   // Mark pump as stopped
  }

  // Handle ultrasonic sensor reading
  if (stateUSensor == false && started && !stopped) {
    activateSolenoid(); // Turn on solenoid (pump starts)
    Serial.println("Solenoid activated, Pump started");
    started = true; // Pump is started
    stopped = false; // Mark pump as running
  } else if (stateUSensor == true && started) {
    deactivateSolenoid(); // Turn off solenoid if liquid height exceeds MAX_FUNNELHEIGHT
    Serial.println("Solenoid deactivated, Pump stopped due to sensor");
    started = false; // Pump is stopped
    stopped = true;  // Mark pump as stopped
  }

  // Handle start button press (start the pump if stopped)
  if (stateStart == LOW && stateStartPrior == HIGH && stopped) {
    if (!started) { // Start pump only if it's not already running
      activateSolenoid(); // Turn on solenoid
      Serial.println("Pump started");
      started = true; // Mark pump as started
      stopped = false; // Mark pump as running
    }
  }

  // Update prior states for debounce comparison
  stateStartPrior = stateStart;
  stateStopPrior = stateStop;
}