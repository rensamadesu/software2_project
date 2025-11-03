#include <Servo.h>

// Arduino pin assignment
#define PIN_IR    0       // IR sensor at Pin A0
#define PIN_LED   9
#define PIN_SERVO 10

#define _DUTY_MIN 600.0   // servo full clock-wise position (0 degree)
#define _DUTY_NEU 1600.0  // servo neutral position (90 degree)
#define _DUTY_MAX 2500.0  // servo full counter-clockwise position (180 degree)

#define _DIST_MIN  100.0  // minimum distance 100mm
#define _DIST_MAX  250.0  // maximum distance 250mm

#define EMA_ALPHA  0.5    // for EMA Filter 

#define LOOP_INTERVAL 20  // Loop Interval (unit: msec)

Servo myservo;
unsigned long last_loop_time;   // unit: msec

float dist_ema = _DIST_MIN;    
int duty = _DUTY_NEU;           

void setup()
{
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW); // LED OFF 
  
  myservo.attach(PIN_SERVO);  
  myservo.writeMicroseconds(duty);
  
  Serial.begin(1000000);    // 1,000,000 bps

  last_loop_time = millis();  
}

void loop()
{
  unsigned long time_curr = millis();
  float a_value, dist_raw;

  // wait until next event time
  if (time_curr < (last_loop_time + LOOP_INTERVAL))
    return;
  last_loop_time = time_curr; // 루프가 실행될 때마다 last_loop_time을 현재 시간으로 갱신

  a_value = analogRead(PIN_IR);

  // a_value가 15.0 미만(스파이크 구간)이면, 계산을 건너뛰고 모터는 현 위치 유지
  if (a_value < 15.0) { 
    digitalWrite(PIN_LED, LOW); // 범위 밖이므로 LED 끄기
   
  } 
  else {
    
    dist_raw = ((6762.0 / (a_value - 9.0)) - 4.0) * 10.0 - 60.0;
  
    // LED 제어 
    if ((dist_raw >= _DIST_MIN) && (dist_raw <= _DIST_MAX)) {
      digitalWrite(PIN_LED, HIGH); // LED ON
    }
    else {  
      digitalWrite(PIN_LED, LOW);  // LED OFF
    }
  
    dist_ema = (EMA_ALPHA * dist_raw) + (1.0 - EMA_ALPHA) * dist_ema;
        
   
    
    float clamped_dist = dist_ema;
    if (clamped_dist < _DIST_MIN) {
        clamped_dist = _DIST_MIN;
    } else if (clamped_dist > _DIST_MAX) {
        clamped_dist = _DIST_MAX;
    }
  
 
    duty = (int)((clamped_dist - _DIST_MIN) * (_DUTY_MAX - _DUTY_MIN) / (_DIST_MAX - _DIST_MIN) + _DUTY_MIN);
    
    myservo.writeMicroseconds(duty); 
  }
  

  // (스파이크가 무시될 땐, dist_raw, ema, duty는 이전 값을 출력함)
  Serial.print("_DUTY_MIN:");  Serial.print(_DUTY_MIN);
  Serial.print(",_DIST_MIN:"); Serial.print(_DIST_MIN);
  Serial.print(",IR:");        Serial.print(a_value);
  Serial.print(",dist_raw:");  Serial.print(dist_raw); 
  Serial.print(",ema:");       Serial.print(dist_ema);
  Serial.print(",servo:");     Serial.print(duty); // 마지막으로 유효했던 duty 값
  Serial.print(",_DIST_MAX:"); Serial.print(_DIST_MAX);
  Serial.print(",_DUTY_MAX:"); Serial.print(_DUTY_MAX);
  Serial.println("");
}
