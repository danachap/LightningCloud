#include <Arduino.h>

// put function declarations here
int myFunction(int, int);

// setup code here, runs once
void setup() {

  // sample function call
  // int result = myFunction(2, 3);

  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}

// function definitions
int myFunction(int x, int y) {
  return x + y;
}
