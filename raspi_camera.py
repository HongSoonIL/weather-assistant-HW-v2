# raspi_camera.py
# 라즈베리파이5용 통합 서버 (카메라 + 노크 감지 - 노이즈 필터링 적용)

import RPi.GPIO as GPIO
import cv2
from picamera2 import Picamera2
from flask import Flask, Response, jsonify, request
from flask_cors import CORS
import threading
import time
import base64
import requests
from datetime import datetime


# ========== 🔥 [중요] 설정 확인 필요 ==========
# PC(백엔드)의 IP주소가 정확한지 다시 한 번 확인해주세요.
BACKEND_URL = "http://10.39.154.49:4000"  # 🔥 실제 PC IP로 변경 필요!
KNOCK_PIN = 17      # 노크 센서 GPIO 핀
FLASK_PORT = 5000   # 라즈베리파이 포트

# ========== Flask 초기화 ==========
app = Flask(__name__)
CORS(app) # 모든 도메인 허용

# ========== 카메라 초기화 (Picamera2) ==========
try:
    picam2 = Picamera2()
    config = picam2.create_preview_configuration(
        main={"format": "RGB888", "size": (1296, 972)}
    )
    picam2.configure(config)
    picam2.start()
    print("📷 카메라 초기화 완료")
except Exception as e:
    print(f"❌ 카메라 초기화 실패: {e}")

# ========== GPIO 설정 (노크 센서) ==========
GPIO.setmode(GPIO.BCM)
# PUD_UP: 평소엔 HIGH(3.3V) 유지, 스위치 닫히면 LOW(0V)
GPIO.setup(KNOCK_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)

# ========== 전역 변수 ==========
is_streaming = True

# ========== MJPEG 스트리밍 생성기 ==========
def generate_frames():
    while True:
        if is_streaming:
            try:
                frame = picam2.capture_array()
                
                # 🔥 [색상 수정] cvtColor 제거
                # 만약 이미지가 여전히 이상하면 아래 주석을 해제하여 다시 변환을 시도해보세요.
                # frame = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR) 
                
                # JPEG 인코딩
                # OpenCV의 imencode는 BGR을 기대하지만, 현재 색상이 반전된다면
                # 변환 없이 바로 넣는 것이 정답일 수 있습니다.
                ret, buffer = cv2.imencode('.jpg', frame)
                if ret:
                    frame_bytes = buffer.tobytes()
                    yield (b'--frame\r\n'
                           b'Content-Type: image/jpeg\r\n\r\n' + frame_bytes + b'\r\n')
            except Exception as e:
                pass 
        time.sleep(0.05) # FPS 조절

# ========== 🔥 노크 감지 스레드 (엣지 디텍션 적용) ==========
def knock_detection_thread():
    print("🎯 노크 감지 스레드 시작 (노이즈 필터링 적용됨)...")
    
    while True:
        try:
            # 1. 엣지 디텍션: 신호가 HIGH에서 LOW로 떨어지는 '순간'만 감지
            # 2. 디바운싱: 한 번 감지 후 300ms(0.3초) 동안은 노이즈 무시 (bouncetime=300)
            # timeout을 주지 않으면 영원히 대기하므로 CPU 점유율이 0에 가까워짐
            GPIO.wait_for_edge(KNOCK_PIN, GPIO.FALLING, bouncetime=300)
            
            # 감지된 경우에만 아래 코드 실행
            print("✊ 노크 감지됨! 백엔드로 신호 전송 시도...")
            
            try:
                # 백엔드에 알림 전송
                res = requests.post(f"{BACKEND_URL}/knock", timeout=3)
                if res.status_code == 200:
                    print("✅ 백엔드 전송 성공")
                else:
                    print(f"⚠️ 백엔드 응답 코드: {res.status_code}")
            except requests.exceptions.RequestException as e:
                print(f"❌ 백엔드 연결 실패: {e}")
            
            # 추가 안정화 대기 (필요 시 조절)
            time.sleep(0.2)
            
        except Exception as e:
            print(f"⚠️ 센서 스레드 오류: {e}")
            time.sleep(1)

# ========== API 엔드포인트 ==========

@app.route('/video_feed')
def video_feed():
    """실시간 스트리밍 주소"""
    return Response(generate_frames(), mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route('/capture', methods=['POST'])
def capture():
    try:
        print("📸 촬영 요청 수신")
        frame = picam2.capture_array()
        _, buffer = cv2.imencode('.jpg', frame)
        img_base64 = base64.b64encode(buffer).decode('utf-8')
        
        return jsonify({
            "status": "success",
            "image": img_base64,
            "timestamp": datetime.now().isoformat()
        }), 200

    except Exception as e:
        print(f"❌ 촬영 오류: {e}")
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/health', methods=['GET'])
def health():
    return jsonify({"status": "ok", "backend_url": BACKEND_URL}), 200

# ========== 서버 실행 ==========
if __name__ == '__main__':
    try:
        # 별도 스레드에서 노크 감지 실행
        t = threading.Thread(target=knock_detection_thread, daemon=True)
        t.start()
        
        print("="*50)
        print(f"🚀 라즈베리파이 서버 실행 중 (Port: {FLASK_PORT})")
        print(f"📡 연결할 백엔드 주소: {BACKEND_URL}")
        print("="*50)
        
        app.run(host='0.0.0.0', port=FLASK_PORT, threaded=True, debug=False)
    finally:
        if 'picam2' in globals():
            picam2.stop()
        GPIO.cleanup()
        print("리소스 정리 완료")