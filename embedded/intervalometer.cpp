/*
  An intervalometer script for Arduino Pro Mini.

  Hardware requirements:
  - Arduino Pro Mini or similar
  - an oled shield for UI
  - a rotary encoder with a button for input
  - an opto-isolated circuitry for sending focus and shutter
    signals to the camera
  - (a lithum battery)
  - (a battery charging circuitry)
  - (a step-up voltage converter)

  Library requirements:
  - ClickEncoder
  - TaskScheduler
  - U8g2lib
  - TimerOne

  @author Peter Kronström
*/

#include <Arduino.h>

#include <SPI.h>  // problems with PlatformIO
#include <U8g2lib.h>
#include <Wire.h>

#include <ClickEncoder.h>
#include <TimerOne.h>

#include <TaskScheduler.h>

#define SW1 4               // Rotary switch
#define CAMERA_FOCUS   A0   // -> optoisolator circuit
#define CAMERA_SHUTTER 13   // -> optoisolator circuit

#define DISPLAY_AUTOSLEEP_MS 15000  // sleep display on idle (after ms)
#define DISPLAY_BLIT_FREQ 100       // redraw display every N ms

// Forward declarations
void takePhoto();
void blitDisplay();
void tick();
void blitActive();
void displaySleep();
void displayWake();

// logic variables
int timelapseInterval = 0;
bool timelapseRunning = false;
bool focus = true;
int shootCount = 0;
bool blit = true;
uint8_t activityToggle = 0;
bool isSleeping = false;

// Library initializations
ClickEncoder *encoder;
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(SCL, SDA, U8X8_PIN_NONE);

Task takePhotoTask(0, TASK_FOREVER, &takePhoto);
Task blitDisplayTask(DISPLAY_BLIT_FREQ, TASK_FOREVER, &blitDisplay);
Task blitActiveTask(250, TASK_FOREVER, &blitActive);
Task sleepTimerTask(DISPLAY_AUTOSLEEP_MS, TASK_ONCE, &displaySleep);

Scheduler scheduler;

void timerIsr() {
  encoder->service();
}

void setup(void) {
  pinMode(SW1, INPUT_PULLUP);
  pinMode(CAMERA_FOCUS, OUTPUT);
  pinMode(CAMERA_SHUTTER, OUTPUT);

  // Encoder in pins 2, 3 (hw int). Switch in 4.
  encoder = new ClickEncoder(3, 2, SW1, 4);

  // Begin SSD1306 128x64 OLED in fast mode
  u8x8.begin();
  u8x8.setFont(u8x8_font_pxplusibmcgathin_r);
  u8x8.setContrast(0);

  // Timer1 is needed for encoder interrupts
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);

  // Initialize the scheduler.
  // takePhotoTask is enabled / disabled by pressing the button (in tick())
  // blitDisplayTask is ran every 100ms if variable blit = true
  scheduler.init();
  scheduler.addTask(takePhotoTask);
  scheduler.addTask(blitDisplayTask);
  scheduler.addTask(blitActiveTask);
  scheduler.addTask(sleepTimerTask);

  blitDisplayTask.enable();
  blitActiveTask.enable();
  sleepTimerTask.enableDelayed(); //
}

void loop(void) {
  tick();               // tick inputs
  scheduler.execute();  // tick scheduler
}

void takePhoto() {
  /* Takes a single photo */
  if (focus) {
    digitalWrite(CAMERA_FOCUS, HIGH);
    delay(10);
    digitalWrite(CAMERA_FOCUS, LOW);
  }
  digitalWrite(CAMERA_SHUTTER, HIGH);
  delay(10);
  digitalWrite(CAMERA_SHUTTER, LOW);
  shootCount++;
  blit = true;
}

void blitActive() {
  if (timelapseRunning) {
    switch(activityToggle) {
      case 0:
        u8x8.drawString(15, 7, "/");
        break;
      case 1:
        u8x8.drawString(15, 7, "|");
        break;
      case 2:
        u8x8.drawString(15, 7, "\\");
        break;
      case 3:
        u8x8.drawString(15, 7, "-");
        break;
    }
    activityToggle++;
    if (activityToggle > 3) activityToggle = 0;
  }
}

void blitDisplay() {
  /* This function is called once every 100 ms by blitDisplayTask */
  /* OLED: 16 cols, 8 rows*/
  if (blit) {   // draw only if necessary
    // interval
    char buf[10];
    sprintf(buf, "%02d:%02d", timelapseInterval/60 ,timelapseInterval%60);
    u8x8.drawString(5, 3, buf);

    //photos taken
    sprintf(buf, "#%03d", shootCount);
    u8x8.drawString(0, 7, buf);

    //ON / OFF ?
    u8x8.drawString(13, 7, timelapseRunning ? "ON " : "OFF");
  }
  blit = false;
}

void tick(void) {
  // Check encoder's button
  if ((encoder->getButton() == ClickEncoder::Clicked)) {
    if (!isSleeping) {
      timelapseRunning = !timelapseRunning;
      if (timelapseRunning) {
        takePhotoTask.setInterval(timelapseInterval * 1000);
        takePhotoTask.enable();
      }
      else {
        takePhotoTask.disable();
        shootCount = 0;
      }
      blit = true;
      sleepTimerTask.restartDelayed();
    }
    else {
      displayWake();
    }
  }

  // Check rotary encoder's delta -> update interval in seconds
  int8_t encoder_value = encoder->getValue();
  if (encoder_value != 0) {
    if (!isSleeping) {
      if (!timelapseRunning) {
        timelapseInterval += encoder_value;
        if (timelapseInterval < 0)
          timelapseInterval = 0;
      }
      sleepTimerTask.restartDelayed();
      blit = true;
    }
    else {
      displayWake();
    }
  }

}

void displaySleep() {
  u8x8.setPowerSave(true);
  isSleeping = true;
}

void displayWake() {
  u8x8.setPowerSave(false);
  isSleeping = false;
  sleepTimerTask.restartDelayed();
}
