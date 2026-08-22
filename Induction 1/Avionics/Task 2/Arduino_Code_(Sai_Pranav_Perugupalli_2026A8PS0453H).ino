// Code for Avionics Task-2 by Sai Pranav Perugupalli - 2026A8PS0453H from EnI, BPHC.

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int BUTTON = 3;              //LCD ---> SDA = A4 & SCL = A5
const int LED = 4;
const int ECHO = 5;
const int TRIG = 6;
const int BUZZER = 7;
const int LDR = A0;

const int lightThreshold = 95;     //Light threshold of ~50% adjusted to Tinkercad Physics
const int distThreshold = 100;     //Distance threshold of 100cm
const int cooldownTime = 5000;     //Time window for dropping the anchor after Storm/Charybdis
bool anchorDropped = false;        //Current state of the anchor
bool lastButtonState = HIGH;       //Default button state = HIGH
unsigned long wreckStartTime = 0;  //Stopwatch
unsigned long lastBlinkTime = 0;   //For blinking LED... Cuz, why not?
bool ledState = LOW;               //Default LED state = LOW
String currentLine1 = "";          //First line displayed on LCD
String currentLine2 = "";          //Second line displayed on LCD

enum SystemState {OPEN_SEA, ANCHOR_DROPPED, WRECKED, STORM, CHARYBDIS};    //States

SystemState currentState = OPEN_SEA;  //Default state at starting the simulation

long getDistance() {                  //Function that outputs distance in cm
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite (TRIG, LOW);
  long duration = pulseIn(ECHO, HIGH, 30000);  //Records the time taken for the pulse to get detected by the ECHO pin
  return duration * 0.0343 / 2;
}

void anchorButtonState() {                     //Toggles the anchor state when you press the push button

  bool currentButtonState = digitalRead(BUTTON);
  if (lastButtonState == HIGH && currentButtonState == LOW) {
  anchorDropped = !anchorDropped;
  }
  lastButtonState = currentButtonState;
}

void blinkLED(int interval) {                  //Blinks LED without using the delay() function
  unsigned long currentMillis = millis();
  if (currentMillis - lastBlinkTime >= interval) {
    lastBlinkTime = currentMillis;
    ledState = !ledState;
    digitalWrite(LED, ledState);
  }
}

void updateLCD(String line1, String line2) {    //Updates LCD cleanly without flicker

  if (line1 != currentLine1 || line2 != currentLine2) {
    lcd.clear();
    
    lcd.setCursor(0, 0);
    lcd.print(line1);
    
    lcd.setCursor(0, 1);
    lcd.print(line2);

    currentLine1 = line1;
    currentLine2 = line2;
  }
}

void stopBlinking() {                           //Stops the LED blinking... Duh...
  digitalWrite(LED, LOW);
  ledState = LOW;
}

void setup() {
//Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(TRIG, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(LDR, INPUT);
}

void loop() {
//Serial.println(analogRead(LDR));
  int lightLevel = map(analogRead(LDR), 54, 974, 1, 100);          //TinkerCAD has some weird rules on the light intensity
//Serial.println(lightLevel);
  int distance = getDistance();                                    //The returned value is stored in the variable -> distance
  bool isStorm = (lightLevel < lightThreshold);                    //isStorm can be True or False
  bool isCharybdis = (distance < distThreshold);                   //Again, can be True or False

  anchorButtonState();                                             //Checks the anchor button

if (currentState != WRECKED) {                                     //When the simulation is not wrecked,
    if (anchorDropped) {                                           //If anchor is dropped, set the state to Anchor_Dropped
      currentState = ANCHOR_DROPPED;
      wreckStartTime = 0;
    }
  	else if (currentState == ANCHOR_DROPPED) {                     //If anchor is not dropped,
		  if (isStorm) {                                               //Lines 105 to 117 were added to the code to fix the issue that, if you withdraw the anchor during Storm and/or Charybdis, the state goes to Open_Sea for a fraction of a second before showing the respective correct state...
        currentState = STORM;
        wreckStartTime = millis();
      } 
      else if (isCharybdis) {
        currentState = CHARYBDIS;
        wreckStartTime = millis();
      } 
      else {
        currentState = OPEN_SEA;
      }
    }
  }

  switch (currentState) {                                          //Switch - Case statements to switch the current state

    case OPEN_SEA:
      stopBlinking();
      noTone(BUZZER);
      updateLCD("OPEN SEA", "SAILING SAFELY");
      
      if (isStorm) {
        currentState=STORM;
        wreckStartTime=millis();
      }
      else if (isCharybdis) {
        currentState=CHARYBDIS;
        wreckStartTime=millis();
      }
      break;

    case ANCHOR_DROPPED:
      stopBlinking();
      noTone(BUZZER);
      updateLCD("ANCHOR DROPPED", "You're Good");
      break;

    case STORM:
      blinkLED(100);
      noTone(BUZZER);
      updateLCD("STORM", "Drop the anchor!");

      if (millis() - wreckStartTime >= cooldownTime) {
        currentState=WRECKED;
      }
      else if (!isStorm) {
        currentState=OPEN_SEA;
        wreckStartTime=0;
      }
      break;

    case CHARYBDIS:
      stopBlinking();
      tone(BUZZER, 440);
      updateLCD("CHARYBDIS", "Drop the anchor!");

      if (millis() - wreckStartTime >= cooldownTime) {
        currentState = WRECKED;
      } 
      else if (!isCharybdis) {
        currentState = OPEN_SEA;
        wreckStartTime = 0;
      }
      break;

    case WRECKED:
      stopBlinking();
      noTone(BUZZER);
      updateLCD("WRECKED", "Restart the simulation");
      break;
  }

	delay(50);                 //Adding this delay made the code run smoother...
}
