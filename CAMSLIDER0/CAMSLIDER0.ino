// Sketch for Diy Camera Slider
// 10/11/2022 by Jobit Joseph for Circuit Digest
// Project homepage: https://circuitdigest.com/
///////////////////////////////////////////////////////////////////////////////////////
//Terms of use
///////////////////////////////////////////////////////////////////////////////////////
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//THE SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////////

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <AccelStepper.h>
#include <MultiStepper.h>
#include "bitmap.h"

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define I2Cspeed 1000000
#define limitSwitch 10
#define PinSW 3
#define PinCLK 2
#define PinDT 4
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
AccelStepper stepper2(1, 8, 7);  // (Type:driver, STEP, DIR)
AccelStepper stepper1(1, 6, 5);
MultiStepper StepperControl;

long gotoposition[2];
int selection;
volatile long XInPoint = 0;
volatile long YInPoint = 0;
volatile long XOutPoint = 0;
volatile long YOutPoint = 0;
volatile long totaldistance = 0;
int flag = 0;
int temp = 0;
int i, j;
unsigned long switch0 = 0;
unsigned long rotary0 = 0;
float setspeed = 200;
float motorspeed;
float timeinsec;
float timeinmins;
volatile boolean TurnDetected;
volatile boolean rotationdirection;

void oledDisplayCenter(const char* text, int x, int y) {
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  display.getTextBounds(text, x, y, &x1, &y1, &width, &height);
  display.setCursor((SCREEN_WIDTH - width) / 2, y);
  display.print(text);  // text to display
}
void Switch() {
  if (flag == 7 && selection == 0 && millis() - switch0 > 500)
  {

    flag = 10;
  }

  else if (millis() - switch0 > 500) {
    flag = flag + 1;
    selection = 1;
  }
  switch0 = millis();
}

void Rotary() {
  delay(75);
  if (digitalRead(PinCLK))
    rotationdirection = digitalRead(PinDT);
  else
    rotationdirection = !digitalRead(PinDT);
  TurnDetected = true;
  delay(75);
}

void setup() {
  Serial.begin(9600);
  stepper1.setMaxSpeed(3000);
  stepper1.setSpeed(200);
  stepper2.setMaxSpeed(3000);
  stepper2.setSpeed(200);

  pinMode(limitSwitch, INPUT_PULLUP);
  pinMode(PinSW, INPUT_PULLUP);
  pinMode(PinCLK, INPUT_PULLUP);
  pinMode(PinDT, INPUT_PULLUP);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  // Create instances for MultiStepper - Adding the 2 steppers to the StepperControl instance for multi control
  StepperControl.addStepper(stepper1);
  StepperControl.addStepper(stepper2);

  attachInterrupt(digitalPinToInterrupt(PinSW), Switch, RISING);  // SW connected to D2
  attachInterrupt(digitalPinToInterrupt(PinCLK), Rotary, RISING);  // CLK Connected to D3

  // display Boot logo
  for (int i = 124; i > 0; i = i - 4) {
    display.drawBitmap(i, 0, CamSlider, 128, 64, 1);
    display.display();
    display.clearDisplay();
  }

  delay(1000);
  display.clearDisplay();

  Home();  // Move the slider to the initial position - homing
}

void Home() {
  stepper1.setMaxSpeed(3000);
  stepper1.setSpeed(200);
  stepper2.setMaxSpeed(3000);
  stepper2.setSpeed(200);
  if (digitalRead(limitSwitch) == 1) {

    for (int i = 92; i > 3; i = i - 4) {
      display.drawBitmap(17, 5, Homing, 92, 27, 1);
      display.drawBitmap(0, 51, slider, 128, 13, 1);
      display.drawBitmap(i, 40, camera, 20, 17, 1);
      display.display();
      display.clearDisplay();
    }
  }
  while (digitalRead(limitSwitch) == 1) {
    stepper1.setSpeed(-3000);
    stepper1.runSpeed();
  }
  delay(20);
  stepper1.setCurrentPosition(0);
  stepper1.moveTo(200);
  while (stepper1.distanceToGo() != 0) {
    stepper1.setSpeed(3000);
    stepper1.runSpeed();
  }
  stepper1.setCurrentPosition(0);
  display.clearDisplay();
}

void SetSpeed() {
  display.clearDisplay();
  while (flag == 6) {
    if (TurnDetected) {
      TurnDetected = false;  // do NOT repeat IF loop until new rotation detected
      if (rotationdirection) {
        setspeed = setspeed + 30;
      }
      if (!rotationdirection) {
        setspeed = setspeed - 30;
        if (setspeed < 0) {
          setspeed = 0;
        }
      }

      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(30, 0);
      display.print("Speed");
      motorspeed = setspeed / 80;
      display.setCursor(5, 16);
      display.print(motorspeed);
      display.print(" mm/s");
      totaldistance = XOutPoint - XInPoint;
      if (totaldistance < 0) {
        totaldistance = totaldistance * (-1);
      } else {
      }
      timeinsec = (totaldistance / setspeed);
      timeinmins = timeinsec / 60;
      display.setCursor(35, 32);
      display.print("Time");
      display.setCursor(8, 48);
      if (timeinmins > 1) {
        display.print(timeinmins);
        display.print(" min");
      } else {
        display.print(timeinsec);
        display.print(" sec");
      }
      display.display();
    }
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(30, 0);
    display.print("Speed");
    motorspeed = setspeed / 80;
    display.setCursor(5, 16);
    display.print(motorspeed);
    display.print(" mm/s");
    totaldistance = XOutPoint - XInPoint;
    if (totaldistance < 0) {
      totaldistance = totaldistance * (-1);
    } else {
    }
    timeinsec = (totaldistance / setspeed);
    timeinmins = timeinsec / 60;
    display.setCursor(35, 32);
    display.print("Time");
    display.setCursor(8, 48);
    if (timeinmins > 1) {
      display.print(timeinmins);
      display.print(" min");
    } else {
      display.print(timeinsec);
      display.print(" sec");
    }
    display.display();
  }
}
void select()
{
  if (TurnDetected)
  {
    TurnDetected = false;
    if (rotationdirection && selection == 0)
    {
      selection = 1;
      Serial.print("selection: ");
      Serial.println(selection);
    }
    if (!rotationdirection && selection == 1)
    {
      selection = 0;
      Serial.print("selection: ");
      Serial.println(selection);
    }
  }
}
void stepperposition(int n) {
  stepper1.setMaxSpeed(3000);
  stepper1.setSpeed(200);
  stepper2.setMaxSpeed(3000);
  stepper2.setSpeed(200);
  if (TurnDetected) {
    TurnDetected = false;  // do NOT repeat IF loop until new rotation detected
    if (n == 1) {
      if (!rotationdirection) {
        if (stepper1.currentPosition() - 500 > 0) {
          stepper1.move(-500);
          while (stepper1.distanceToGo() != 0) {
            stepper1.setSpeed(-6000);
            stepper1.runSpeed();
          }
        } else {
          while (stepper1.currentPosition() != 0) {
            stepper1.setSpeed(-6000);
            stepper1.runSpeed();
          }
        }
      }

      if (rotationdirection) {
        if (stepper1.currentPosition() + 500 < 30000) {
          stepper1.move(500);
          while (stepper1.distanceToGo() != 0) {
            stepper1.setSpeed(6000);
            stepper1.runSpeed();
          }
        } else {
          while (stepper1.currentPosition() != 30000) {
            stepper1.setSpeed(6000);
            stepper1.runSpeed();
          }
        }
      }
    }
    if (n == 2) {
      if (rotationdirection) {
        stepper2.move(-50);
        while (stepper2.distanceToGo() != 0) {
          stepper2.setSpeed(-3000 / 2);
          stepper2.runSpeed();
        }
      }
      if (!rotationdirection) {
        stepper2.move(50);
        while (stepper2.distanceToGo() != 0) {
          stepper2.setSpeed(3000 / 2);
          stepper2.runSpeed();
        }
      }
    }
  }
}


void loop() {
  //Begin Setup
  if (flag == 0) {
    display.clearDisplay();
    display.drawBitmap(30, 12, BeginSetup, 58, 37, 1);
    display.display();
    setspeed = 200;
  }

  //SetXin
  if (flag == 1) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.drawBitmap(0, 51, slider, 128, 13, 1);
    display.drawBitmap(4, 40, camera, 20, 17, 1);
    oledDisplayCenter("SLIDE IN", 0, 5);
    display.display();
    while (flag == 1) {
      stepperposition(1);    
    }
    XInPoint = stepper1.currentPosition();
  }
  //SetYin
  if (flag == 2) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.drawBitmap(0, 51, slider, 128, 13, 1);
    display.drawBitmap(4, 40, camera, 20, 17, 1);
    oledDisplayCenter("PAN IN", 0, 5);
    display.display();
    while (flag == 2) {
      stepperposition(2);
    }
    stepper2.setCurrentPosition(0);
    YInPoint = stepper2.currentPosition();
  }
  //SetXout
  if (flag == 3) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.drawBitmap(0, 51, slider, 128, 13, 1);
    display.drawBitmap(92, 40, camera, 20, 17, 1);
    oledDisplayCenter("SLIDE OUT", 0, 5);
    display.display();
    while (flag == 3) {
      stepperposition(1);
      Serial.println(stepper1.currentPosition());
    }
    XOutPoint = stepper1.currentPosition();
  }
  //SetYout
  if (flag == 4) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.drawBitmap(0, 51, slider, 128, 13, 1);
    display.drawBitmap(92, 40, camera, 20, 17, 1);
    oledDisplayCenter("PAN OUT", 0, 5);
    display.display();
    while (flag == 4) {
      stepperposition(2);
    }
    YOutPoint = stepper2.currentPosition();
    display.clearDisplay();

    // Go to IN position
    gotoposition[0] = XInPoint;
    gotoposition[1] = YInPoint;
    display.clearDisplay();
    display.setCursor(8, 28);
    display.println(" Preview  ");
    display.display();
    stepper1.setMaxSpeed(3000);
    StepperControl.moveTo(gotoposition);
    StepperControl.runSpeedToPosition();
  }

  //Display Set Speed
  if (flag == 5) {
    display.clearDisplay();
    display.setCursor(8, 28);
    oledDisplayCenter("Set Speed", 0, 20);
    display.display();
  }
  //Change Speed
  if (flag == 6) {
    display.clearDisplay();
    SetSpeed();
  }
  //DisplayStart
  if (flag == 7) {
    display.clearDisplay();
    display.setCursor(30, 27);
    oledDisplayCenter("Start", 0, 5); 
    display.drawBitmap(20, 35, XMARK, 24, 24, 1);
    display.drawBitmap(84, 35, TMARK, 24, 24, 1);
    if(selection == 0)
    {
      display.drawRoundRect(18, 33, 28 , 28, 2, WHITE);
    }
    else 
    {
      display.drawRoundRect(82, 33, 28 , 28, 2, WHITE);
    }
    display.display();
    
    select();
  }
  //Start
  if (flag == 8) {
    display.clearDisplay();
    display.setCursor(20, 27);
    oledDisplayCenter("Running", 0, 20);
    display.display();
    Serial.println(XInPoint);
    Serial.println(XOutPoint);
    Serial.println(YInPoint);
    Serial.println(YOutPoint);

    gotoposition[0] = XOutPoint;
    gotoposition[1] = YOutPoint;
    stepper1.setMaxSpeed(setspeed);
    StepperControl.moveTo(gotoposition);
    StepperControl.runSpeedToPosition();
    
    flag = flag + 1;
  }
  //Slide Finish
  if (flag == 9) {
    display.clearDisplay();
    display.setCursor(24, 26);
    oledDisplayCenter("Finish", 0, 20);
    display.display();
  }
  //Return to start
  if (flag == 10) {
    display.clearDisplay();
    Home();
    flag = 0;
  }
}