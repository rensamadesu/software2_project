#define PIN_LED 7
unsigned int count;
void setup() {
  pinMode(PIN_LED, OUTPUT);
  Serial.begin(115200);
  while (!Serial) {
  ;
  }
}

void loop() {
  digitalWrite(PIN_LED, 0);
  delay(1000);
  
  for (count = 0; count < 5; count++) {
    digitalWrite(PIN_LED, 1);
    delay(100);
    digitalWrite(PIN_LED, 0);
    delay(100);
  }
  
  digitalWrite(PIN_LED, 1);
  while (1) { } // infinite loop
}
