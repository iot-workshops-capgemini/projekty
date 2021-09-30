/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink
*/

static const int LED_OUT = 21;
static const int LIGHT_IN = 18;
int buttonState = 0;

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  // initialize digital pin LED_OUT as an output.
  pinMode(LED_OUT, OUTPUT);
  // initialize digital pin LIGHT_IN as an output.
  pinMode(LIGHT_IN, INPUT);
  digitalWrite(LED_OUT, HIGH);
  delay(1000);
}

// the loop function runs over and over again forever
void loop() {
  buttonState = digitalRead(LED_IN);
  Serial.println(buttonState);
  
  if (buttonState == HIGH) {
    digitalWrite(LED_OUT, HIGH);   // turn the LED on (HIGH is the voltage level)
  } else {
    digitalWrite(LED_OUT, LOW);    // turn the LED off by making the voltage LOW
  }
  delay(500);   // half second slow-down
}
