// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12
#define PIN_ECHO 13

// configurable parameters
#define SND_VEL 346.0      // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 25        // sampling interval (unit: msec)
#define PULSE_DURATION 10  // ultra-sound Pulse Duration (unit: usec)
#define _DIST_MIN 100      // minimum distance to be measured (unit: mm)
#define _DIST_MAX 300      // maximum distance to be measured (unit: mm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0) // maximum echo waiting time (unit: usec)
#define SCALE (0.001 * 0.5 * SND_VEL)     // coefficent to convert duration to distance

#define _EMA_ALPHA 0.5     // EMA weight of new sample (range: 0 to 1) 
#define N 30              // 중앙값을 계산할 샘플의 개수. 과제에 따라 3, 10, 30으로 변경.

// global variables
unsigned long last_sampling_time; // unit: msec
float dist_ema;                   // EMA distance
float samples[N];                 // 최근 N개의 측정값을 저장할 배열
float dist_median;                // 계산된 중앙값을 저장할 변수

void sort_array(float arr[], int size) { // sort_array 라는 이름의 함수를 정의. 
  for (int i = 0; i < size - 1; i++) { // 이중 반복문을 사용. 배열의 모든 요소를 서로 한 번씩 비교하기 위한 구조. 
    for (int j = 0; j < size - i - 1; j++) { 
      if (arr[j] > arr[j + 1]) { // 만약 왼쪽 값이 오른쪽 값보다 크다면 
        float temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp; // 라는 임시 변수를 이용해 두 값의 자리를 서로 맞바꿈. 
      }
    }
  }
}

void setup() {
  // initialize GPIO pins
  pinMode(PIN_LED,OUTPUT);
  pinMode(PIN_TRIG,OUTPUT);
  pinMode(PIN_ECHO,INPUT);
  digitalWrite(PIN_TRIG, LOW);

  // initialize serial port
  Serial.begin(57600);

  // samples 배열과 EMA 값을 적절한 초기값으로 설정
  float initial_dist = (_DIST_MIN + _DIST_MAX) / 2.0; 
  for (int i = 0; i < N; i++) {
    samples[i] = initial_dist;
  }
  dist_ema = initial_dist;
}

void loop() {
  float dist_raw;
  
  // wait until next sampling time.
  if (millis() < last_sampling_time + INTERVAL) // 주기적인 측정을 위한 시간 관리 코드 
    return;

  // get a distance reading from the USS
  dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);

  // --- 중앙값 필터 구현 시작 ---
  
  for (int i = 0; i < N - 1; i++) { // 새로운 측정값을 samples 배열에 추가함. 오래된 값은 버림. 
    samples[i] = samples[i + 1];
  }
  samples[N - 1] = dist_raw; // 배열의 비어있는 맨 마지막 칸에 방금 측정한 dist_raw 값을 집어넣음. 
  
  float sorted_samples[N]; // 중앙값 계산. 정렬 작업을 위한 임시 복사본 배열을 만듬. 
  for (int i = 0; i < N; i++) { // 원본 samples 배열의 값을 복사본 배열에 그대로 복사!
    sorted_samples[i] = samples[i]; 
  }
  sort_array(sorted_samples, N); // 위에서 만든 정렬 시키는 함수를 불러와 복사본 배열을 정렬시킴. 
  dist_median = sorted_samples[N / 2]; // 정렬된 배열의 정가운데에 있는 값을 dist_median 변수에 저장. 이것이 바로 '중앙값'

  // EMA 값은 중앙값 필터를 거친 값을 기반으로 계산
  dist_ema = _EMA_ALPHA * dist_median + (1 - _EMA_ALPHA) * dist_ema; // 중앙값으로 1차 필터링된 dist_median값을
                                                                     // 이용하여 EMA 필터링을 한 번 더 수행하고
                                                                     // 그 결과를 dist_ema에 저장.
                                                                     
                                                                     // emak-1 = k-1이 아니라 1번째 값을 의미 
                                                                     // 알파의 수치를 올릴수록 그래프가 더 요동을 침
                                                                     // 지금 측정값에 더 가중을 하기 때문에.
                                                                     // 반대로 낮으면 그래프는 완만하지만 
                                                                     // 부정확한 데이터가 될 확률이 높음
                                                                     // (내가 손을 굉장히 빠르게 요동친다는 가정)
  
  // 과제에 형식에 맞게 시리얼 포트로 값을 출력 
  Serial.print("Min:");      Serial.print(_DIST_MIN);
  Serial.print(",raw:");     Serial.print(dist_raw);
  Serial.print(",ema:");     Serial.print(dist_ema);
  Serial.print(",median:");  Serial.print(dist_median);
  Serial.print(",Max:");     Serial.print(_DIST_MAX);
  Serial.println("");

  // LED 제어는 중앙값이 dist_min과 dist_max의 거리를 벗어났는지 확인하고 벗어났으면 off, 범위면 on. 
  if ((dist_median < _DIST_MIN) || (dist_median > _DIST_MAX))
    digitalWrite(PIN_LED, 1);      // LED OFF
  else
    digitalWrite(PIN_LED, 0);      // LED ON

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
  
  // Pulse duration to distance conversion example (target distance = 17.3m)
  // - pulseIn(ECHO, HIGH, timeout) returns microseconds (음파의 왕복 시간)
  // - 편도 거리 = (pulseIn() / 1,000,000) * SND_VEL / 2 (미터 단위)
  //   mm 단위로 하려면 * 1,000이 필요 ==>  SCALE = 0.001 * 0.5 * SND_VEL
  //
  // - 예, pusseIn()이 100,000 이면 (= 0.1초, 왕복 거리 34.6m)
  //        = 100,000 micro*sec * 0.001 milli/micro * 0.5 * 346 meter/sec
  //        = 100,000 * 0.001 * 0.5 * 346
  //        = 17,300 mm  ==> 17.3m
}
