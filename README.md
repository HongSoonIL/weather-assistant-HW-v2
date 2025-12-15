# 🔮 Lumee 2.0 - 똑똑한 감성 날씨 어시스턴트

> "사용자 취향과 건강 민감도를 반영해, 가장 필요한 날씨 정보를 선별해주는 AI 날씨 비서"


## 📌 프로젝트 개요

- 개발 기간: 2025.09.19 ~ 2025.12.19
- 개발 목적: 2025 캡스톤 디자인 프로젝트 1 과제전



## 🚀 주요 기능

- 🔍 LLM 기반 자연어 질문 인식: `"마스크 써야 해?"`, `"서울 비와?"`, `"우산 챙길까?"`
- 🌐 사용자 위치 기반 자동 날씨 제공
- 🎯 사용자 민감 요소/취미 기반 맞춤형 조언
- 📊 기온/미세먼지 그래프 시각화
- 🧠 LLM에게 실시간 날씨 정보를 넘겨 최종 응답 생성



## 💻 링크

- DEMO: [Lumee🔮](https://hongsoonil.github.io/weather-assistant-frontend/)
- PREVIEW: [Demo Video: Lumee⛅🔮](https://youtu.be/PBAn7sUd3rI?feature=shared)
- WORKSPACE: [Team Project: Lumee⛅🔮](https://www.notion.so/Aurora-Studio-Lumee-2-0-2030e11d4cf280c38f63de812d22286a?source=copy_link)



## 👥 팀원

| 이름 | 역할 |
| --- | --- |
| 홍순일　<br/> (팀장) | PM, 개발팀 |
| 정지은 | 개발팀 |
| 문수현 | 디자인팀 |
| 이지윤 | 디자인팀 |
| 조현지 | 디자인팀 |



## 🛠️ 사용 기술 스택

| 분야 | 기술 |
| --- | --- |
| 프론트엔드 | React, Tailwind CSS, Recharts |
| 백엔드 | Node.js |
| AI 모델 | Gemini API (Google Generative AI) |
| 데이터 API | OpenWeather(날씨), Ambee(꽃가루), Google Geocoding(위치) |
| DB | Firebase Realtime Database |
| 배포 | GithubPage(FE), Render (BE) |
| 생성형 AI 영상 제작 | Midjourney(구슬 컨셉 디자인), Runway(구슬 영상 생성) |



## 📂 프로젝트 구조

### 1. 리포지토리

[**Front : weather-assistant-frontend-v2**](https://github.com/HongSoonIL/weather-assistant-frontend-v2.git)

[**Back : weather-assistant-backend-v2**](https://github.com/HongSoonIL/weather-assistant-backend-v2.git)

[**HW : weather-assistant-HW-v2**](https://github.com/HongSoonIL/weather-assistant-HW-v2.git)

### 2. 파일구조

```markdown
📦 Mobile-Magicians
┣ 📂 weather-assistant-frontend
┃  ┣ 📂 weather-assistant
┃  ┃  ┗ 📂 src
┃  ┃    ┣ 📂 screens
┃  ┃    ┣ 📂 services
┃  ┃    ┣ 📜 App.js
┃  ┃    ┗ ...
┃  ┗ ...
┣ 📂 weather-assistant-backend
┃  ┗ 📂 backend
┃    ┣ 🔒 .env
┃    ┣ 📜 server.js
┃    ┣ 📜 tools.js
┃    ┣ 📜 geminiUtils.js
┃    ┣ 📜 weatherUtils.js
┃    ┣ 📜 userProfileUtils.js
┃    ┗ ...
┗ ...
📂 weather-assistant-HW
  ┗ 📂 Claud_LED
    ┣ 📜 Claud_LED.ino
  ┗ 📂 Claud_LED2
    ┣ 📜 Claud_LED.ino
  ┣ 📜 raspi_camera.py
```



## ⚙️ 로컬 실행 방법

### 라즈베리파이 실행

```
cd Desktop
source Lumee/bin/activate
python raspi_camera.py
```

