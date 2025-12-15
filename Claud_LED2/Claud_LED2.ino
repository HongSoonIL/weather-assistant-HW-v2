/*
 * Lumee 소리 전용 아두이노 (COM7 연결)
 * 역할: PC로부터 USB 시리얼로 숫자를 받아 DFPlayer 재생
 */
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// DFPlayer 연결 (RX, TX)
SoftwareSerial mp3Serial(10, 11); 
DFRobotDFPlayerMini myDFPlayer;

void setup() {
  Serial.begin(9600);     // PC(Node.js)와 통신
  mp3Serial.begin(9600);  // DFPlayer와 통신

  Serial.println("Sound System Initializing...");

  if (!myDFPlayer.begin(mp3Serial)) {
    Serial.println("DFPlayer Error!");
    // while(true); // 에러 시 멈추고 싶으면 주석 해제
  }
  
  myDFPlayer.volume(25); // 볼륨 설정 (0~30)
  Serial.println("Sound System Ready on COM14");
}

void loop() {
  // PC에서 데이터가 오면 읽음
  if (Serial.available()) {
    int soundId = Serial.parseInt(); // 정수형으로 읽기
    
    if (soundId > 0) {
      Serial.print("Playing Track: ");
      Serial.println(soundId);
      
      // 폴더 01의 soundId 번째 파일 재생
      myDFPlayer.playFolder(1, soundId); 
    }
  }
}