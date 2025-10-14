#define PIN_LED  9
#define PIN_TRIG 12
#define PIN_ECHO 13

#define SND_VEL 346.0
#define INTERVAL 25
#define PULSE_DURATION 10
#define _DIST_MIN 100
#define _DIST_MAX 300

#define TIMEOUT ((INTERVAL / 2) * 1000.0)
#define SCALE (0.001 * 0.5 * SND_VEL)

#define _EMA_ALPHA 0.5  // EMA weight
#define N 30            // Median filter window size (3, 10, 30 등 실험)

// global variables
unsigned long last_sampling_time;
float dist_prev = _DIST_MAX;
float dist_ema;
float samples[N];
int index_buf = 0;

void setup() {
pinMode(PIN_LED, OUTPUT);
pinMode(PIN_TRIG, OUTPUT);
pinMode(PIN_ECHO, INPUT);
digitalWrite(PIN_TRIG, LOW);
Serial.begin(57600);
}

void loop() {
float dist_raw, dist_filtered, dist_median;

if (millis() < last_sampling_time + INTERVAL)
return;

// ① 초음파 센서 거리 측정
dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);

// ② 범위 기반 필터
if ((dist_raw == 0.0) || (dist_raw > _DIST_MAX)) {
    dist_filtered = dist_prev;
} else if (dist_raw < _DIST_MIN) {
    dist_filtered = dist_prev;
} else {
    dist_filtered = dist_raw;
    dist_prev = dist_raw;
}

// ③ EMA 필터
dist_ema = _EMA_ALPHA * dist_filtered + (1.0 - _EMA_ALPHA) * dist_ema;

// ④ 중위수(Median) 필터
samples[index_buf] = dist_filtered;
index_buf = (index_buf + 1) % N;
dist_median = medianFilter(samples, N);

// ⑤ 시리얼 출력
Serial.print("Min:"); Serial.print(_DIST_MIN);
Serial.print(",raw:"); Serial.print(min(dist_raw, _DIST_MAX + 100));
Serial.print(",ema:"); Serial.print(min(dist_ema, _DIST_MAX + 100));
Serial.print(",median:"); Serial.print(min(dist_median, _DIST_MAX + 100));
Serial.print(",Max:"); Serial.print(_DIST_MAX);
Serial.println("");

// ⑥ LED 제어
if ((dist_raw < _DIST_MIN) || (dist_raw > _DIST_MAX))
digitalWrite(PIN_LED, 1);
else
digitalWrite(PIN_LED, 0);

last_sampling_time += INTERVAL;
}

// 초음파 거리 측정 함수
float USS_measure(int TRIG, int ECHO) {
digitalWrite(TRIG, HIGH);
delayMicroseconds(PULSE_DURATION);
digitalWrite(TRIG, LOW);
return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; // mm 단위 반환
}

// 중위수(Median) 필터 함수
float medianFilter(float arr[], int size) {
float temp[size];
memcpy(temp, arr, sizeof(temp));
for (int i = 0; i < size - 1; i++) {
for (int j = i + 1; j < size; j++) {
if (temp[i] > temp[j]) {
float t = temp[i];
temp[i] = temp[j];
temp[j] = t;
}
}
}
return temp[size / 2]; // 정렬 후 가운데 값 반환
}
