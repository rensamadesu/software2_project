#include <Servo.h>

#define PIN_SERVO 10
#define PIN_TRIG 12
#define PIN_ECHO 13

Servo myservo;

long duration;
int distance;
bool isOpen = false;
unsigned long openTime = 0;

void setup() {
  Serial.begin(57600);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_DECHO, INPUT);

  myservo.attach(PIN_SERVO);
  myservo.write(80);  // 초기 상태: 닫힘 (80도)
}

void loop() {
  // 초음파 센서로 거리 측정
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  duration = pulseIn(PIN_DECHO, HIGH);
  distance = duration * 0.034 / 2;  // cm 변환

  Serial.print("Distance: ");
  Serial.println(distance);

  // 차량 감지 && 닫혀 있으면 열기
  if (distance > 0 && distance <= 10 && !isOpen) {
    myservo.write(0);  // 열림 (0도)
    isOpen = true;
    openTime = millis();  // 열린 시간 기록
  }

  // 열려 있고, 5초 지났으면 닫기
  if (isOpen && millis() - openTime >= 5000) {
    myservo.write(80);  // 닫힘 (80도)
    isOpen = false;
  }

  delay(100);  // 너무 자주 측정하지 않도록 딜레이
}
