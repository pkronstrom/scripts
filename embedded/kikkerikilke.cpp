#include <Console.h>

//http://playground.arduino.cc/uploads/Code/FSM.zip
#include <FiniteStateMachine.h>

//https://github.com/adafruit/Adafruit_NeoPixel
#include <Adafruit_NeoPixel.h>

#define BLINK_INTERVAL 500
#define LASER_THRESHOLD 250
#define LASER_TRIGGER_MARGIN 1000 // Safe-zone after laser trigger in ms

#define SLEEP_TIMEOUT 300000 // timeout 5 min

#define PURKKAPWM_MAX 100
#define PURKKAPWM_DUTY 97

/* PIN DEFINITIONS */
// button inputs
#define P1G 2
#define P1R 3
#define P2G 4
#define P2R 5
#define RES 6

// Button LEDs -> Do we need a 74HC595?
#define P1G_LED 7
#define P1R_LED 8
#define P2G_LED 9
#define P2R_LED 10
#define RES_LED 11

// LDR sensors for reading lasers
#define LZR1 A0
#define LZR2 A1

// Enable lasers pin
#define LZR_EN 13

// Goal counter LED's
#define GOAL_COUNTER1 A2
#define GOAL_COUNTER2 A3

/* GLOBAL VARIABLES */

// State machine states
State Sleep = State(sleepStateEnter, sleepStateUpdate, dummyStateExit);
State Ready = State(readyStateEnter, readyStateUpdate, dummyStateExit);
State GameOn = State(gameOnStateEnter, gameOnStateUpdate, dummyStateExit);
State GameEnd = State(gameEndStateEnter, gameEndStateUpdate, dummyStateExit);

// State machine with the initial 'Sleep' state
FSM kikkeriStateMachine = FSM(Sleep);

Adafruit_NeoPixel p1Counter = Adafruit_NeoPixel(10, GOAL_COUNTER1, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel p2Counter = Adafruit_NeoPixel(10, GOAL_COUNTER2, NEO_RGB + NEO_KHZ800);

bool readyStateP1Ready = false;
bool readyStateP2Ready = false;
bool toggle = false;

bool lazorsDuty = PURKKAPWM_MAX;
bool lazorState = true;
uint8_t purkkaPwm = 0;


char debounce = 0;    // debounce for inputs
uint8_t buttons;      // global for inputs. Volatile?
unsigned long previousMillis = 0;
unsigned long currentTime;
unsigned long sleepCounter = 0;

uint8_t p1Score, p2Score = 0;

void setup() {
  
  //Bridge.begin();   // required by YUN
  //Console.begin();  // required by YUN
  
  // set digital pins 2...6 as inputs
  DDRD = DDRD | 0b11111100;
  // TODO: enable internal pullups, if necessary


  // set lights as outputs
  pinMode(P1G_LED, OUTPUT);
  pinMode(P1R_LED, OUTPUT);
  pinMode(P2G_LED, OUTPUT);
  pinMode(P2R_LED, OUTPUT);
  pinMode(RES_LED, OUTPUT);

  // Initialize goal counters
  p1Counter.begin();
  p2Counter.begin();
  p1Counter.show();
  p2Counter.show();

  // set laser LDR sensors as input
  pinMode(LZR1, INPUT);
  pinMode(LZR2, INPUT);
  pinMode(LZR_EN, OUTPUT);

  // Wifi console requires this
  //while (!Console);

  Serial.begin(57600); // debug
  
}

void loop() {

  readInputs();
  digitalWrite(LZR_EN, HIGH);
  kikkeriStateMachine.update();
  
  // always check the reset button
  if (checkButton(RES)) {
    Serial.println("Reset pressed!");
    kikkeriStateMachine.transitionTo(Ready);
    delay(500);
  }
  
}


void readInputs() {
  /* The function polls the inputs once and saves 
     the state into a 'buttons' variable.
  
    an alternative to current implementation:
    https://blog.adafruit.com/2009/10/20/example-code-for-multi-button-checker-with-debouncing/

  */

  // Purkka-PWM for the lasors
  // Dim the lasers by 100th, because the photoresistor is
  // otherwise way too slow.
  purkkaPwm++;
  if (purkkaPwm > PURKKAPWM_MAX)
    purkkaPwm = 0;
  if (purkkaPwm > PURKKAPWM_DUTY)
    lazorState = true;
  else
    lazorState = false;

  digitalWrite(LZR_EN, lazorState);

  // reset button counter on next loop iteration.
  buttons = 0x00;

  // Check out the lazords. Digital read for nao.
  buttons = (~PINC & 0b00000011);
  
  if (debounce > 0)
    debounce--;

  if (!debounce) {
    // bits 0..5 -> inputs D1..D6
    buttons += (PIND & 0b01111100);
  }

  // if any of the buttons are pressed
  // set debounce timer to 100 ms
  if (buttons)
    debounce = 100;
}

int checkButton(int btn_number) {
  return (buttons & (1 << btn_number));
}

int checkButtons() {
  // check the buttons, not including lasers
  return (buttons & 0b01111100);
}

void lightsOff() {
  digitalWrite(P1R_LED, LOW);
  digitalWrite(P1G_LED, LOW);
  digitalWrite(P2R_LED, LOW);
  digitalWrite(P2G_LED, LOW);
  digitalWrite(RES_LED, LOW);
}

void lightsOn() {
  digitalWrite(P1R_LED, HIGH);
  digitalWrite(P1G_LED, HIGH);
  digitalWrite(P2R_LED, HIGH);
  digitalWrite(P2G_LED, HIGH);
  digitalWrite(RES_LED, HIGH);
}

void drawGoals() {
  // Show goals on 10 led strips

  // check that the goals don't exceed 10
  for(uint8_t i=0; i<p1Score; i++) {
    p1Counter.setPixelColor(i, 0, 255, 0);
    p1Counter.show();
  }
  for(uint8_t i=0; i<p2Score; i++) {
    p2Counter.setPixelColor(i, 255, 0, 0);
    p2Counter.show();
  }
}

/* STATE MACHINE STATES */

void dummyStateExit() {
  sleepCounter = millis();
  return;
}

void sleepStateEnter() {
  lightsOff();
  Serial.println("sleepStateEnter");
}

void sleepStateUpdate() {
  /*
  // advance to readyState on ANY input press
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 30000) {
    previousMillis = currentMillis;
    digitalWrite(RES_LED, HIGH);
    delay(250);
    digitalWrite(RES_LED, LOW);
  }
  */
  if (checkButtons()) {
    kikkeriStateMachine.transitionTo(Ready);
  }
}

void readyStateEnter() {
  readyStateP1Ready = false;
  readyStateP2Ready = false;
  previousMillis = 0;
  
  lightsOff();
  
  Serial.println("readyStateEnter");
}

void readyStateUpdate() {
  // Blink Green buttons, when both have been pressed, advance to gameOnState

  // blink Green button LEDs
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= BLINK_INTERVAL) {
    previousMillis = currentMillis;

    if (!readyStateP1Ready)
      digitalWrite(P1G_LED, toggle);
    if (!readyStateP2Ready)
      digitalWrite(P2G_LED, toggle);

    toggle = !toggle;
  }

  // when both green buttons have been pressed, move to GameOn state
  if (readyStateP1Ready && readyStateP2Ready)
    kikkeriStateMachine.transitionTo(GameOn);

  // Set the 'green button pressed' variables for both players, if pressed
  // The player is now 'ready', the LED should now have a constant light
  if (checkButton(P1G)) {
    Serial.println("P1 Ready!");
    readyStateP1Ready = true;
    digitalWrite(P1G_LED, HIGH);
  }

  if (checkButton(P2G)) {
    Serial.println("P2 Ready!");
    readyStateP2Ready = true;
    digitalWrite(P2G_LED, HIGH);
  }

}

void gameOnStateEnter() {
  p1Score = 0;
  p2Score = 0;
  drawGoals();
  
  lazorsDuty = PURKKAPWM_DUTY;

  Serial.println("gameOnStateEnter");
  
  lightsOn();
  delay(2500);
  lightsOff();
  digitalWrite(RES_LED, HIGH);  // light the RESET button
}

void gameOnStateUpdate() {
  // Goal counters show goals
  // Red / Green buttons adjust goals

  uint8_t goalTemp = p1Score + p2Score;

  // Check Player 1 buttons
  if (checkButton(P1G)) {
    Serial.println("P1Score++");
    digitalWrite(P1G_LED, HIGH);
    p1Score++;
    delay(250);
    digitalWrite(P1G_LED, LOW);
  }
  else if (checkButton(P1R)) {
    Serial.println("P1Score--");
    digitalWrite(P1R_LED, HIGH);
    if (p1Score > 0)
      p1Score--;
    delay(250);
    digitalWrite(P1R_LED, LOW);
  }

  // Check Player 2 buttons
  if (checkButton(P2G)) {
    Serial.println("P2Score++");
    digitalWrite(P2G_LED, HIGH);
    p2Score++;
    delay(250);
    digitalWrite(P2G_LED, LOW);
  }
  else if (checkButton(P2R)) {
    Serial.println("P2Score--");
    digitalWrite(P2R_LED, HIGH);
    if (p2Score > 0)
      p2Score--;
    delay(250);
    digitalWrite(P2R_LED, LOW);
  }

  // track goals on laser trigger
    if ((buttons & (1 << 0))) {
      p1Score++;
      Serial.println("P1 Scores!");
      delay(250);
    }
    else if ((buttons & (1 << 1))) {
      p2Score++;
      Serial.println("P2 Scores!");
      delay(250);
    }

  if (p1Score + p2Score != goalTemp) {
    // On score change, update goal counters
    drawGoals();
  }

  // on 10 goals, advance to gameEndState
  if ((p1Score >= 10) || (p2Score >= 10)) {
    kikkeriStateMachine.transitionTo(GameEnd);
  }

  // On Reset go back to readyState
  
  if (checkButton(RES)) {
    kikkeriStateMachine.transitionTo(Ready);
    Serial.println("RESET!!");
    delay(250);
  }

}

void gameEndStateEnter() {
  lazorsDuty = PURKKAPWM_DUTY;

  Serial.println("gameEndStateEnter");
  Serial.print("P1: ");
  Serial.println(p1Score);
  Serial.print("P2: ");
  Serial.println(p2Score);
  
  lightsOn();
  delay(2500);
  lightsOff();
}

void gameEndStateUpdate() {
  // Show goals on goal counters, maybe goal animation
  drawGoals();

  // blink all button LED's
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= BLINK_INTERVAL) {
    previousMillis = currentMillis;

    digitalWrite(P1G_LED, toggle);
    digitalWrite(P1R_LED, toggle);
    digitalWrite(P2G_LED, toggle);
    digitalWrite(P2R_LED, toggle);
    digitalWrite(RES_LED, toggle);

    toggle = !toggle;
  }

  // on ANY button press, advance to Ready state
  if (checkButtons()) {
    kikkeriStateMachine.transitionTo(Ready);
    Serial.println("Transitioning to ReadyState");
  }

}
