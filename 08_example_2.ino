// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12   // sonar sensor TRIGGER
#define PIN_ECHO 13   // sonar sensor ECHO
// configurable parameters
#define SND_VEL 346.0     // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 25      // sampling interval (unit: msec)
#define PULSE_DURATION 10 // ultra-sound Pulse Duration (unit: usec)
#define _DIST_MIN 100.0   // minimum distance to be measured (unit: mm)
#define _DIST_MID 200.0   // middle distance to be measured (unit: mm)
#define _DIST_MAX 300.0   // maximum distance to be measured (unit: mm)
#define TIMEOUT ((INTERVAL / 2) * 1000.0) // maximum echo waiting time (unit: usec)
#define SCALE (0.001 * 0.5 * SND_VEL) // coefficent to convert duration to distance
unsigned long last_sampling_time;   // unit: msec
  float light_range; // 밝기 수준 


void setup() {
  // initialize GPIO pins
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);  // sonar TRIGGER
  pinMode(PIN_ECHO, INPUT);   // sonar ECHO
  digitalWrite(PIN_TRIG, LOW);  // turn-off Sonar
  // initialize serial port
  Serial.begin(115200);
}

void loop() {
  float distance; // 거리 변수 선언 
  // wait until next sampling time. // polling
  // millis() returns the number of milliseconds since the program started.
  //    will overflow after 50 days.
  if (millis() < (last_sampling_time + INTERVAL))
    return;
  
  distance = USS_measure(PIN_TRIG, PIN_ECHO); // read distance
  if ((distance == 0.0) || (distance > _DIST_MAX)) {
      distance = _DIST_MAX + 10.0;    // Set Higher Value
      analogWrite(PIN_LED, 255);       // LED 끔 
  } else if (distance < _DIST_MIN) {
      distance = _DIST_MIN - 10.0;    // Set Lower Value
      analogWrite(PIN_LED, 255);       // LED 끔 
  } else if (_DIST_MIN < distance && distance < _DIST_MID){    // 거리가 100 초과 200 미만일때
      light_range = 255 - (255 * (distance - _DIST_MIN) / 100);
      analogWrite(PIN_LED, light_range);       // LED ON
      Serial.println(light_range);
  } else if (_DIST_MID < distance && distance < _DIST_MAX){    // 거리가 200 초과 300 미만일때
      light_range = (255 * (distance - _DIST_MID) / 100);
      analogWrite(PIN_LED, light_range);       // LED ON
      Serial.println(light_range);
  }
    else {    // In desired Range
      digitalWrite(PIN_LED, 0);       // LED 켜짐 
  }
 
  // output the distance to the serial port
  Serial.print("Min:");        Serial.print(_DIST_MIN);
  Serial.print(",distance:");  Serial.print(distance);
  Serial.print(",Max:");       Serial.print(_DIST_MAX);
  Serial.println("");
  // update last sampling time
  last_sampling_time += INTERVAL;
}
// get a distance reading from USS. return value is in millimeter.
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; // unit: mm
}
