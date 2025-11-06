/*
 * Smart Air Quality Monitor - v5.3 (Stable)
 * Fixes:
 * - Removed token.h dependency
 * - Corrected Hostname + OTA hash alignment
 * - WebSocket clients authenticate only once
 * - Improved logging for debugging
 */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// -------------------- CONFIG --------------------

// WiFi credentials
static const char* SSID = "realme123";
static const char* WIFI_PASS = "rinurinaaa";

// Device hostname
static const char* DEVICE_HOSTNAME = "air-quality";

// Token used for WebSocket + HTTP auth
static const char* WS_BEARER_TOKEN = "f2c7c683b9154bb7de99ca6a73b40a791053f1c968ce2d735e879fe9259ed54e";

// Ports
static const uint16_t HTTP_PORT = 80;
static const uint16_t SENSOR_INTERVAL_MS = 2000;

// OLED settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDRESS 0x3C

// Sensor pins
#define SEALEVELPRESSURE_HPA (1013.25)
#define DUST_ILED_PIN 27
#define DUST_AOUT_PIN 34

// Rate limiting
const unsigned long WS_MSG_WINDOW_MS = 10000;
const uint16_t WS_MSG_MAX_PER_WINDOW = 20;

// -------------------- HARDWARE OBJECTS --------------------
Adafruit_BME680 bme;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
AsyncWebServer server(HTTP_PORT);
AsyncWebSocket ws("/ws");

// -------------------- STATE --------------------
unsigned long lastSensorMillis = 0;

// -------------------- STRUCTS --------------------
struct WSClientRate {
  uint32_t clientId;
  unsigned long windowStart;
  uint16_t count;
};

struct ClientAuthState {
  uint32_t clientId;
  bool authenticated;
};

#define MAX_WS_CLIENTS 8
WSClientRate clientRates[MAX_WS_CLIENTS];
ClientAuthState authStates[MAX_WS_CLIENTS];

// -------------------- FUNCTION DECLARATIONS --------------------
void initWiFi();
void initOTA();
void initMDNS();
void initSensorsAndDisplay();
void initWebSocket();
void sendSensorDataToClients();
float readDustSensor();
bool checkAndIncrementRate(uint32_t clientId);
bool isClientAuthenticated(uint32_t clientId);
void setClientAuthenticated(uint32_t clientId, bool state);

// -------------------- SETUP --------------------
void setup() {
  Serial.begin(115200);
  delay(10);

  Wire.begin();
  initSensorsAndDisplay();

  pinMode(DUST_ILED_PIN, OUTPUT);
  digitalWrite(DUST_ILED_PIN, LOW);

  // ✅ ADC Configuration for ESP32 Dust Sensor
  analogReadResolution(12); // Full 12-bit resolution (0–4095)
  analogSetPinAttenuation(DUST_AOUT_PIN, ADC_11db); // Allows 0–3.3V input range

  initWiFi();
  initOTA();

  if (!MDNS.begin(DEVICE_HOSTNAME)) {
    Serial.println("❌ mDNS start failed!");
  } else {
    Serial.printf("✅ mDNS responder started: %s.local\n", DEVICE_HOSTNAME);
  }

  initWebSocket();

  // REST endpoint for diagnostics
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!request->hasHeader("Authorization")) {
      request->send(401, "application/json", "{\"error\":\"missing auth\"}");
      return;
    }

    const AsyncWebHeader* h = request->getHeader("Authorization");
    String val = h ? h->value() : "";

    if (!val.startsWith("Bearer ") || val.substring(7) != String(WS_BEARER_TOKEN)) {
      request->send(403, "application/json", "{\"error\":\"forbidden\"}");
      return;
    }

    StaticJsonDocument<256> doc;
    doc["uptime_ms"] = millis();
    doc["wifi_rssi"] = WiFi.RSSI();
    doc["hostname"] = DEVICE_HOSTNAME;

    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
  });

  server.begin();
  Serial.println("✅ HTTP server started");
}

// -------------------- LOOP --------------------
void loop() {
  ArduinoOTA.handle();

  if (millis() - lastSensorMillis >= SENSOR_INTERVAL_MS) {
    lastSensorMillis = millis();
    if (WiFi.status() == WL_CONNECTED) sendSensorDataToClients();
  }
}

// -------------------- INIT FUNCTIONS --------------------
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, WIFI_PASS);
  Serial.printf("📶 Connecting to %s...\n", SSID);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(200);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("✅ WiFi connected, IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("⚠️ WiFi connection failed (continuing offline).");
  }
}

void initOTA() {
  ArduinoOTA.setHostname(DEVICE_HOSTNAME);
  // MD5 hash for "S0me$tr0ngP@ss!"
  ArduinoOTA.setPasswordHash("a277474f554bda2cf6054c6a2940183d");

  ArduinoOTA.onStart([]() { Serial.println("🔄 OTA: Start"); });
  ArduinoOTA.onEnd([]() { Serial.println("\n✅ OTA: End"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("📦 OTA Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t err) {
    Serial.printf("❌ OTA Error[%u]: ", err);
    if (err == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (err == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (err == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (err == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (err == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();
  Serial.println("✅ OTA ready");
}

void initSensorsAndDisplay() {
  if (!bme.begin()) {
    Serial.println("⚠️ BME680 not found!");
  } else {
    bme.setTemperatureOversampling(BME680_OS_2X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_2X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150);
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    Serial.println("⚠️ SSD1306 init failed");
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Smart Air Quality Monitor");
    display.display();
  }
}

// -------------------- HELPER FUNCTIONS --------------------
bool checkAndIncrementRate(uint32_t clientId) {
  unsigned long now = millis();
  for (int i = 0; i < MAX_WS_CLIENTS; ++i) {
    if (clientRates[i].clientId == clientId) {
      if (now - clientRates[i].windowStart > WS_MSG_WINDOW_MS) {
        clientRates[i].windowStart = now;
        clientRates[i].count = 1;
        return true;
      } else {
        if (clientRates[i].count < WS_MSG_MAX_PER_WINDOW) {
          clientRates[i].count++;
          return true;
        }
        return false;
      }
    }
  }

  for (int i = 0; i < MAX_WS_CLIENTS; ++i) {
    if (clientRates[i].clientId == 0) {
      clientRates[i].clientId = clientId;
      clientRates[i].windowStart = now;
      clientRates[i].count = 1;
      return true;
    }
  }

  return false;
}

bool isClientAuthenticated(uint32_t clientId) {
  for (int i = 0; i < MAX_WS_CLIENTS; ++i) {
    if (authStates[i].clientId == clientId) return authStates[i].authenticated;
  }
  return false;
}

void setClientAuthenticated(uint32_t clientId, bool state) {
  for (int i = 0; i < MAX_WS_CLIENTS; ++i) {
    if (authStates[i].clientId == 0 || authStates[i].clientId == clientId) {
      authStates[i].clientId = clientId;
      authStates[i].authenticated = state;
      return;
    }
  }
}

// -------------------- WEBSOCKET --------------------
void onWsEvent(AsyncWebSocket * serverPtr, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t * data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("🔌 WS client connected: %u\n", client->id());
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("❌ WS client disconnected: %u\n", client->id());
    for (int i = 0; i < MAX_WS_CLIENTS; ++i)
      if (clientRates[i].clientId == client->id()) clientRates[i].clientId = 0;
    setClientAuthenticated(client->id(), false);
  } else if (type == WS_EVT_DATA) {
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    if (!(info->final && info->opcode == WS_TEXT)) return;

    String msg((char*)data, len);
    Serial.printf("💬 WS[%u]: %s\n", client->id(), msg.c_str());

    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, msg);
    if (err) {
      client->text("{\"error\":\"bad_json\"}");
      return;
    }

    const char* typeStr = doc["type"] | "";
    if (strcmp(typeStr, "auth") == 0) {
      const char* token = doc["token"] | "";
      if (strcmp(token, WS_BEARER_TOKEN) == 0) {
        setClientAuthenticated(client->id(), true);
        client->text("{\"ok\":true,\"msg\":\"auth_ok\"}");
        Serial.printf("✅ Client %u authenticated\n", client->id());
      } else {
        client->text("{\"error\":\"invalid_token\"}");
        client->close();
        Serial.printf("🚫 Client %u failed auth\n", client->id());
      }
      return;
    }

    // Require authentication for all other messages
    if (!isClientAuthenticated(client->id())) {
      client->text("{\"error\":\"unauthorized\"}");
      client->close();
      Serial.printf("⚠️ Unauthorized message from %u\n", client->id());
      return;
    }

    // Optional: handle other message types later
    client->text("{\"ok\":true,\"msg\":\"noop\"}");
  }
}

void initWebSocket() {
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  // Basic test page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    const char html[] PROGMEM =
    "<!DOCTYPE html><html><body><h3>Air Monitor</h3><script>"
    "let ws=new WebSocket('ws://'+location.host+'/ws');"
    "ws.onopen=()=>ws.send(JSON.stringify({type:'auth',token:'f2c7c683b9154bb7de99ca6a73b40a791053f1c968ce2d735e879fe9259ed54e'}));"
    "ws.onmessage=e=>console.log(e.data);"
    "</script></body></html>";
    request->send(200, "text/html", html);
  });
}

// -------------------- SENSOR & DISPLAY --------------------
void sendSensorDataToClients() {
  StaticJsonDocument<256> doc;

  float t = NAN, h = NAN, p = NAN, g = NAN, dust = NAN;
  if (bme.performReading()) {
    t = bme.temperature;
    h = bme.humidity;
    p = bme.pressure / 100.0;
    g = bme.gas_resistance / 1000.0;
  }

  dust = readDustSensor();

  float hum_score = constrain(100.0 - h, 0.0, 100.0);
  float gas_score = constrain(g * 10.0, 0.0, 100.0);
  float iaq = (hum_score * 0.25f) + (gas_score * 0.75f);
  float inverted_iaq = constrain((100.0f - iaq) * 2.0f, 0.0f, 200.0f);

  if (!isnan(t)) doc["temp"] = t;
  if (!isnan(h)) doc["hum"] = h;
  if (!isnan(p)) doc["pres"] = p;
  if (!isnan(g)) doc["gas"] = g;
  doc["iaq"] = inverted_iaq;
  doc["dust"] = dust;
  doc["ts"] = millis();

  char out[256];
  size_t n = serializeJson(doc, out, sizeof(out));

  ws.textAll(out);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.printf("T: %.1f C\nH: %.0f %%\nG: %.2f kOhm\nD: %.0f\nIAQ: %.1f\n",
                 isnan(t) ? 0.0f : t,
                 isnan(h) ? 0.0f : h,
                 isnan(g) ? 0.0f : g,
                 dust,
                 inverted_iaq);
  display.display();
}

float readDustSensor() {
  const int samples = 5;  // Average multiple readings for noise reduction
  unsigned long sum = 0;

  for (int i = 0; i < samples; i++) {
    digitalWrite(DUST_ILED_PIN, HIGH);
    delayMicroseconds(280);
    sum += analogRead(DUST_AOUT_PIN);
    delayMicroseconds(40);
    digitalWrite(DUST_ILED_PIN, LOW);
    delayMicroseconds(9680); // Sharp GP2Y1010 timing
  }

  float avgRaw = sum / samples;
  float voltage = (avgRaw / 4095.0f) * 3.3f; // Convert ADC to voltage (0–3.3V)
  float dust = max(0.0f, (voltage - 0.6f) * 100.0f); // Convert voltage to dust density

  return dust;
}

