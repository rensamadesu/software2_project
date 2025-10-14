#include <Servo.h>

#define PIN_SERVO 10
#define PIN_TRIG 12
#define PIN_ECHO 13

Servo myservo;

long duration;
int distance;
bool isOpen = false;
unsigned long openTime = 0;

// Sigmoid 함수
float sigmoid(float x) {
  return 1.0 / (1.0 + exp(-x));
}

// 부드럽게 열기 (닫힘 → 열림: 80 → 0)
void smoothOpen() {
  for (float x = -6.0; x <= 6.0; x += 0.3) {
    float y = sigmoid(x);
    int angle = 80 - (y * 80);  // 각도 반전 적용
    myservo.write(angle);
    delay(30);
  }
}

// 부드럽게 닫기 (열림 → 닫힘: 0 → 80)
void smoothClose() {
  for (float x = 6.0; x >= -6.0; x -= 0.3) {
    float y = sigmoid(x);
    int angle = 80 - (y * 80);  // 각도 반전 적용
    myservo.write(angle);
    delay(30);
  }
}

void setup() {
  Serial.begin(57600);  // 시리얼 속도 설정

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_DECHO, INPUT);

  myservo.attach(PIN_SERVO);
  myservo.write(80);  // 시작 시 닫힘 상태 (80도)
}

void loop() {
  // 초음파 거리 측정
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
    openTime = millis();    // 먼저 시간 기록
    smoothOpen();           // 부드럽게 열기
    isOpen = true;
  }

  // 열려 있고, 5초가 지났으면 닫기
  if (isOpen && millis() - openTime >= 5000) {
    smoothClose();          // 부드럽게 닫기
    isOpen = false;
  }

  delay(100);  // 센서 측정 속도 조절
}
