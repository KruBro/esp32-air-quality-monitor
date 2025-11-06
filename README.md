🏠 Indoor Air Quality Monitor — ESP32 Firmware
██╗███╗   ██╗██████╗  ██████╗  ██████╗ ██████╗ 
██║████╗  ██║██╔══██╗██╔═══██╗██╔════╝ ██╔══██╗
██║██╔██╗ ██║██████╔╝██║   ██║██║  ███╗██████╔╝
██║██║╚██╗██║██╔══██╗██║   ██║██║   ██║██╔══██╗
██║██║ ╚████║██║  ██║╚██████╔╝╚██████╔╝██║  ██║
╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝ ╚═════╝  ╚═════╝ ╚═╝  ╚═╝

     Indoor Air Quality Monitor — ESP32 Firmware


Firmware for an ESP32-powered IoT indoor air monitoring system using:

✅ BME680 (Temp, Humidity, Pressure, VOC)
✅ Sharp GP2Y Dust Sensor
✅ SSD1306 OLED
✅ AsyncWebServer + WebSocket
✅ Secure authentication
✅ OTA Updates
✅ mDNS

📦 Project Structure
indoor_air_quality/
│── src/
│   └── main.cpp
│── include/
│   ├── secrets.h          # Not committed
│   ├── secrets.h.example  # Template
│   └── token.h
│── .gitignore
│── platformio.ini
└── README.md

🚀 Features Snapshot

Real-time WebSocket sensor streaming

OLED display output

Secure bearer-token authentication

Over-the-Air firmware updates

REST API for diagnostics

mDNS support (air-quality.local)

Non-blocking async server

🔧 Hardware Connections

(unchanged; kept concise)

✅ Setup Guide

A complete step-by-step process for users.

✅ 1. Clone the repository
git clone https://github.com/<user>/<repo>.git
cd indoor_air_quality

✅ 2. Install PlatformIO

VS Code → Extensions → PlatformIO IDE

✅ 3. Create your secrets file

Copy:

include/secrets.h.example  →  include/secrets.h


Edit:

#define WIFI_SSID       "YourWiFi"
#define WIFI_PASSWORD   "Password123"
#define OTA_PASSWORD    "mysuperpassword"
#define DEVICE_HOSTNAME "air-quality"


⚠️ Do NOT commit secrets.h.

✅ 4. Configure WebSocket token

Edit include/token.h:

#define WS_BEARER_TOKEN "your_64byte_token_here"


React frontend must use the SAME token.

✅ 5. Assign Static IP to ESP32

Required for OTA.

Example used:

192.168.1.5

✅ 6. Build & Upload (USB the first time)
pio run --target upload

✅ 7. OTA Upload (Next time)

ESP32 must be online.

pio run --target upload


Works because upload_protocol = espota

✅ 8. Web Dashboard Setup (React)

Set in frontend:

const ESP32_WEBSOCKET_URL = "ws://192.168.1.5/ws";
const WS_BEARER_TOKEN = "your_token";

🔐 REST API

✅ /status (requires bearer token)

📊 WebSocket Format

Sent from ESP32:

{
  "temp": 24.5,
  "hum": 63.2,
  "pres": 1005.3,
  "gas": 21030,
  "iaq": 51,
  "dust": 380
}

🧪 Testing the Dust Sensor (GP2Y)

Needs stable 5V

LED pin must be pulsed

ADC pin = GPIO 34/35/36

Should not be dirty or blocked

Prefers clean airflow

📄 License

MIT License.

🙌 Contributions

PRs and issues welcome!
