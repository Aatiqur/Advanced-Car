// WebServerRoutes.ino — status + control endpoints
#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>

extern WebServer server;
extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;
extern const char* AP_NAME;
extern const float TARGET_DISTANCE_M;
extern bool autoRunActive;
extern unsigned long motionStartMillis;
extern MotionState motion;
extern const char INDEX_HTML[];

float readAverageEncoderDistanceM(long &leftTicksOut, long &rightTicksOut);
void beginForwardOneMeterRun();
void stopAutoRun();
void resetMotionEstimates();

String buildJsonStatus() {
  long leftTicks = 0;
  long rightTicks = 0;
  float encoderDistance = readAverageEncoderDistanceM(leftTicks, rightTicks);
  IPAddress currentIp = (WiFi.getMode() == WIFI_AP) ? WiFi.softAPIP() : WiFi.localIP();
  String currentSsid = (WiFi.getMode() == WIFI_AP) ? AP_NAME : WIFI_SSID;

  String json = "{";
  json += "\"running\":"          + String(autoRunActive ? "true" : "false") + ",";
  json += "\"wifiMode\":\""        + String(WiFi.getMode() == WIFI_AP ? "AP" : "STA") + "\",";
  json += "\"ip\":\""              + currentIp.toString() + "\",";
  json += "\"ssid\":\""            + currentSsid + "\",";
  json += "\"leftTicks\":"         + String(leftTicks) + ",";
  json += "\"rightTicks\":"        + String(rightTicks) + ",";
  json += "\"encoderDistanceM\":"  + String(encoderDistance, 3) + ",";
  json += "\"targetDistanceM\":"   + String(TARGET_DISTANCE_M, 3) + ",";
  json += "\"rollDeg\":"           + String(motion.rollDeg, 2) + ",";
  json += "\"pitchDeg\":"          + String(motion.pitchDeg, 2) + ",";
  json += "\"yawDeg\":"            + String(motion.yawDeg, 2) + ",";
  json += "\"velocityMps\":"       + String(motion.velocityMps, 3) + ",";
  json += "\"imuDistanceM\":"      + String(motion.distanceM, 3) + ",";
  json += "\"imuAccelMps2\":"      + String(motion.accelMagnitudeMps2, 3) + ",";
  json += "\"rollOffsetDeg\":"     + String(motion.rollOffsetDeg, 3) + ",";
  json += "\"pitchOffsetDeg\":"    + String(motion.pitchOffsetDeg, 3) + ",";
  json += "\"motionTimeMs\":"      + String(autoRunActive ? (millis() - motionStartMillis) : 0);
  json += "}";
  return json;
}

void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
}

void handleStatus() {
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", buildJsonStatus());
}

void handleStart() {
  beginForwardOneMeterRun();
  server.send(200, "application/json", "{\"ok\":true,\"message\":\"started\"}");
}

void handleStop() {
  stopAutoRun();
  server.send(200, "application/json", "{\"ok\":true,\"message\":\"stopped\"}");
}

void handleReset() {
  stopAutoRun();
  resetEncoderTicks();
  resetMotionEstimates();
  server.send(200, "application/json", "{\"ok\":true,\"message\":\"reset\"}");
}

void handleCalibrate() {
  int duration = 10;
  if (server.hasArg("duration")) {
    duration = server.arg("duration").toInt();
    if (duration <= 0) duration = 10;
  }
  calibrateMpuBiasDuration(duration);
  String out = "{\"ok\":true,\"message\":\"calibrated\",\"duration\":" + String(duration) + "}";
  server.send(200, "application/json", out);
}

void connectWiFi() {
  Serial.printf("Connecting to SSID '%s'...\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startMillis = millis();
  const unsigned long timeoutMs = 30000;
  while (WiFi.status() != WL_CONNECTED && millis() - startMillis < timeoutMs) {
    delay(500);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected to WiFi. IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("RSSI: ");
    Serial.println(WiFi.RSSI());
    return;
  }

  Serial.print("WiFi connect failed, status=");
  Serial.println(WiFi.status());
  Serial.println("WiFi STA failed, starting fallback AP.");

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_NAME);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
}
