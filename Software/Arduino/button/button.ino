#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

class Button
{
public:
  Button(int pButtonId, int pStatePin, int pLedPin);

  void init();
  void update();
  void switchState();
  void sendSerial();

  bool active() const;

protected:
  int mId;
  int mStatePin;
  int mLedPin;
  bool mInit;
  bool mActive;

  int mPreviousState;
  int mOnColor1[3] = {0, 55, 0}; 
  int mOnColor2[3] = {0, 255, 0};
  int mOffColor1[3] = {55, 0, 0};
  int mOffColor2[3] = {255, 0, 0};

  unsigned int mColorPulseInterval = 2000;

  Adafruit_NeoPixel mLed;

  unsigned int mPrevUpdateTime = 0;
  unsigned long mUpdateInterval = 50;
};

Button::Button(int pButtinId, int pStatePin, int pLedPin):
  mId(pButtinId),
  mStatePin(pStatePin),
  mLedPin(pLedPin),
  mInit(false),
  mActive(false)
{}

void 
Button::init()
{
  if(mInit == false)
  {
    // Button State Init
    pinMode(mStatePin, INPUT_PULLUP);
    mPreviousState = digitalRead(mStatePin);

    // Button Led Init
    mLed = Adafruit_NeoPixel(1, mLedPin, NEO_GRB + NEO_KHZ800);
    mLed.begin();

    mInit = true;
  }
}

void 
Button::update()
{
  if(mInit == false)
    return;
  
  unsigned long updateTime = millis();
  if(updateTime - mPrevUpdateTime < mUpdateInterval)
  return;

  mPrevUpdateTime = updateTime;
  int buttonState = digitalRead(mStatePin);
  
  // Detect state change (edge detection)
  if(buttonState != mPreviousState)
  {
    // If the new state is LOW, the button was just pressed
    if(buttonState == LOW)
    {
      switchState();
      delay(50); // Simple debounce to prevent rapid firing
    }
    // Save the current state for the next loop iteration
    mPreviousState = buttonState;
  }

  // Update NeoPixel

  unsigned long pulseTime = updateTime % mColorPulseInterval;
  double pulseScale = 0.5 + sin((double)pulseTime / (double)mColorPulseInterval * PI) * 0.5;

  int ledColor[3];

  if(mActive == true)
  {
    ledColor[0] = int((double)mOnColor1[0] * pulseScale + ((double)mOnColor2[0] - (double)mOnColor1[0]) * (1.0 - pulseScale));
    ledColor[1] = int((double)mOnColor1[1] * pulseScale + ((double)mOnColor2[1] - (double)mOnColor1[1]) * (1.0 - pulseScale));
    ledColor[2] = int((double)mOnColor1[2] * pulseScale + ((double)mOnColor2[2] - (double)mOnColor1[2]) * (1.0 - pulseScale));
  }
  else
  {
    ledColor[0] = int((double)mOffColor1[0] * pulseScale + ((double)mOffColor2[0] - (double)mOffColor1[0]) * (1.0 - pulseScale));
    ledColor[1] = int((double)mOffColor1[1] * pulseScale + ((double)mOffColor2[1] - (double)mOffColor1[1]) * (1.0 - pulseScale));
    ledColor[2] = int((double)mOffColor1[2] * pulseScale + ((double)mOffColor2[2] - (double)mOffColor1[2]) * (1.0 - pulseScale));
  }
  
  mLed.setPixelColor(0, mLed.Color(ledColor[0], ledColor[1], ledColor[2]));
  mLed.show(); 
}

void
Button::switchState()
{
  mActive = !mActive;

  sendSerial();
}

void
Button::sendSerial()
{
  Serial.print(mId);
  Serial.print(" ");
  Serial.println(mActive ? 1 : 0);
}

bool
Button::active() const
{
  return mActive;
}

//Button button(1, 3, 4);
//Button button(2, 3, 4);
Button button(3, 3, 4);

void setup() 
{
  Serial.begin(9600);

  button.init();

  // debug
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() 
{
  button.update();

  if (button.active() == true)
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else
  {
    digitalWrite(LED_BUILTIN, LOW);
  }
}
