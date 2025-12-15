/*
 * Lumee 날씨 LED 시스템 (개선된 파싱 로직 적용)
 * 아두이노 R3 + HM-10 블루투스 + LED 스트립
 */

#include <FastLED.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// LED 설정
#define LED_PIN 6
#define NUM_LEDS 12
#define BRIGHTNESS 200
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

// 블루투스 설정 (HM-10)
// RX: 2번 핀 (HM-10의 TX와 연결)
// TX: 3번 핀 (HM-10의 RX와 연결)
SoftwareSerial bluetooth(2, 3); 

// DFPlayer Mini 설정 - 10번(RX), 11번(TX)
// 아두이노 10번 <-> 모듈 TX
// 아두이노 11번 <-> 모듈 RX (저항 1kΩ 연결 권장)
SoftwareSerial mp3Serial(10, 11);
DFRobotDFPlayerMini myDFPlayer;

// 현재 LED 상태 구조체
struct LEDState {
  uint8_t r, g, b;
  String effect;
  int duration;
  int priority;
  int soundId;
} currentState;

// 효과 타이머 및 상태 변수
unsigned long lastEffectUpdate = 0;
bool ledOn = true;
int lastSoundId = -1;

// 블루투스 데이터 버퍼
String inputString = "";
bool stringComplete = false;

void setup() {
  Serial.begin(9600);     // PC 연결용 시리얼
  bluetooth.begin(9600);  // 블루투스 모듈용 시리얼

  // FastLED 초기화
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  
  // DFPlayer 초기화
  Serial.println(F("Initializing DFPlayer..."));
  if (!myDFPlayer.begin(mp3Serial)) {
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    // 멈추지 않고 진행하도록 주석 처리하거나, 필요시 while(true); 사용
  }
  myDFPlayer.volume(5);

  // 초기 상태: 대기 중 (부드러운 파랑 호흡 효과)
  currentState = {100, 149, 237, "breathe", 2000, 5};
  
  Serial.println("Lumee LED System Ready");
  Serial.println("Waiting for Bluetooth data...");
  
  showStartupAnimation();
}

void loop() {
  // 블루투스 데이터 수신
  if (bluetooth.available()) {
    // 데이터를 줄바꿈 문자까지 읽음 (타임아웃 1초 기본값)
    String data = bluetooth.readStringUntil('\n');
    data.trim(); // 앞뒤 공백 제거
    
    if (data.length() > 0) {
      Serial.print("Received: ");
      Serial.println(data);
      
      if (data.startsWith("{") && data.endsWith("}")) {
        parseWeatherData(data);
      } else {
        Serial.println("Ignored: Not a valid JSON object");
      }
    }
  }
  
  // LED 효과 업데이트
  updateLEDEffect();
  FastLED.show();
  
  // 너무 빠른 루프 방지
  delay(15);
}

// ========== 데이터 파싱 함수 (개선됨) ==========
// 입력 예시: {"r":70,"g":130,"b":180,"effect":"solid","duration":0,"priority":5,"message":"추움","s":2}
void parseWeatherData(String data) {
  // 각 키값을 찾아서 파싱
  int val;
  String strVal;

  // R값 파싱
  if (parseJsonInt(data, "\"r\":", val)) {
    currentState.r = val;
  }
  
  // G값 파싱
  if (parseJsonInt(data, "\"g\":", val)) {
    currentState.g = val;
  }
  
  // B값 파싱
  if (parseJsonInt(data, "\"b\":", val)) {
    currentState.b = val;
  }
  
  // Effect 파싱
  if (parseJsonString(data, "\"effect\":\"", strVal)) {
    currentState.effect = strVal;
  }
  
  // Duration 파싱
  if (parseJsonInt(data, "\"duration\":", val)) {
    currentState.duration = val;
  }
  
  // Priority 파싱
  if (parseJsonInt(data, "\"priority\":", val)) {
    currentState.priority = val;
  }

  // Sound ID ("s": 값) 파싱
  if (parseJsonInt(data, "\"s\":", val)) {
    currentState.soundId = val;
  } else {
    currentState.soundId = 0; // 값이 없으면 0으로 초기화
  }

  Serial.println("--- Parsed Data ---");
  Serial.print("Color: "); Serial.print(currentState.r); Serial.print(","); Serial.print(currentState.g); Serial.print(","); Serial.println(currentState.b);
  Serial.print("Effect: "); Serial.println(currentState.effect);
  Serial.println("-------------------");

  // 소리 재생 로직
  // SD카드의 '01' 폴더 안에 '001.mp3', '002.mp3' 형태로 파일이 있어야 합니다.
  if (currentState.soundId > 0) {
    myDFPlayer.playFolder(1, currentState.soundId);
    Serial.print("Playing sound track: ");
    Serial.println(currentState.soundId);
  }

  // 상태 업데이트 시 효과 리셋
  lastEffectUpdate = millis();
  ledOn = true;
  
  // 즉시 반응을 위해 한 번 실행
  updateLEDEffect();
  FastLED.show();
}

// JSON 문자열에서 정수값 추출 헬퍼 함수
bool parseJsonInt(String data, String key, int &result) {
  int keyPos = data.indexOf(key);
  if (keyPos == -1) return false;
  
  int startPos = keyPos + key.length();
  int endPos = data.indexOf(",", startPos);
  if (endPos == -1) endPos = data.indexOf("}", startPos); // 마지막 항목일 경우
  
  if (endPos != -1) {
    String valStr = data.substring(startPos, endPos);
    valStr.trim(); // 공백 제거
    result = valStr.toInt();
    return true;
  }
  return false;
}

// JSON 문자열에서 문자열 값 추출 헬퍼 함수
bool parseJsonString(String data, String key, String &result) {
  int keyPos = data.indexOf(key);
  if (keyPos == -1) return false;
  
  int startPos = keyPos + key.length();
  int endPos = data.indexOf("\"", startPos);
  
  if (endPos != -1) {
    result = data.substring(startPos, endPos);
    return true;
  }
  return false;
}

// ========== LED 효과 업데이트 ==========
void updateLEDEffect() {
  unsigned long currentTime = millis();
  
  if (currentState.effect == "solid") {
    solidColor();
  }
  else if (currentState.effect == "fast_blink") {
    if (currentTime - lastEffectUpdate > currentState.duration) {
      ledOn = !ledOn;
      lastEffectUpdate = currentTime;
      ledOn ? solidColor() : clearLEDs();
    }
  }
  else if (currentState.effect == "slow_blink") {
    if (currentTime - lastEffectUpdate > currentState.duration) {
      ledOn = !ledOn;
      lastEffectUpdate = currentTime;
      ledOn ? solidColor() : clearLEDs();
    }
  }
  else if (currentState.effect == "rain") {
    rainEffect();
  }
  else if (currentState.effect == "lightning") {
    lightningEffect();
  }
  else if (currentState.effect == "sparkle") {
    sparkleEffect();
  }
  else if (currentState.effect == "breathe") {
    breatheEffect();
  }
  else if (currentState.effect == "gradient") {
    gradientEffect();
  }
  else if (currentState.effect == "wave") {
    waveEffect();
  }
  else if (currentState.effect == "pulse") {
    pulseEffect();
  }
  else {
    // 알 수 없는 효과면 solid로 처리
    solidColor();
  }
}

// ========== 효과 함수들 ==========

void solidColor() {
  fill_solid(leds, NUM_LEDS, CRGB(currentState.r, currentState.g, currentState.b));
}

void clearLEDs() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
}

void rainEffect() {
  // 물방울이 떨어지는 효과
  fadeToBlackBy(leds, NUM_LEDS, 20);
  
  if (random(100) > 70) {
    int pos = random(NUM_LEDS);
    leds[pos] = CRGB(currentState.r, currentState.g, currentState.b);
  }
}

void lightningEffect() {
  // 번개 효과
  if (random(100) > 95) {
    fill_solid(leds, NUM_LEDS, CRGB(255, 255, 200));
    FastLED.show();
    delay(50);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(random(50, 200));
    fill_solid(leds, NUM_LEDS, CRGB(255, 255, 200));
    FastLED.show();
    delay(30);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  }
}

void sparkleEffect() {
  // 눈 내리는 반짝임
  fadeToBlackBy(leds, NUM_LEDS, 10);
  
  if (random(100) > 80) {
    int pos = random(NUM_LEDS);
    leds[pos] = CRGB(255, 250, 250);
  }
}

void breatheEffect() {
  // 호흡 효과 (부드러운 페이드)
  float breath = (exp(sin(millis() / 2000.0 * PI)) - 0.36787944) * 108.0;
  uint8_t brightness = map(breath, 0, 255, 50, 255);
  
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(
      (currentState.r * brightness) / 255,
      (currentState.g * brightness) / 255,
      (currentState.b * brightness) / 255
    );
  }
}

void gradientEffect() {
  // 무지개 그라데이션 (쾌적 상태)
  uint8_t hue = (millis() / 50) % 255;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(hue + (i * 5), 255, 200);
  }
}

void waveEffect() {
  // 물결 효과 (습도 높음)
  for (int i = 0; i < NUM_LEDS; i++) {
    uint8_t brightness = sin8(millis() / 10 + i * 10);
    leds[i] = CRGB(
      (currentState.r * brightness) / 255,
      (currentState.g * brightness) / 255,
      (currentState.b * brightness) / 255
    );
  }
}

void pulseEffect() {
  // 펄스 효과 (자외선 경고)
  uint8_t brightness = beatsin8(30, 100, 255);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(
      (currentState.r * brightness) / 255,
      (currentState.g * brightness) / 255,
      (currentState.b * brightness) / 255
    );
  }
}

// ========== 시작 애니메이션 ==========
void showStartupAnimation() {
  // 전원 켜짐 시 무지개 효과
  for (int hue = 0; hue < 255; hue += 5) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(hue + (i * 10), 255, 200);
    }
    FastLED.show();
    delay(20);
  }
  
  // 페이드 아웃
  for (int brightness = 200; brightness >= 0; brightness -= 10) {
    FastLED.setBrightness(brightness);
    FastLED.show();
    delay(30);
  }
  
  FastLED.setBrightness(BRIGHTNESS);
  clearLEDs();
  FastLED.show();
}