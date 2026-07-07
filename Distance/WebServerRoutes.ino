// Shared declarations for the split Distance project.
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
extern MotionState rawMotion;
extern MotionState kalmanMotion;
extern KalmanSettings kalmanSettings;
extern const char INDEX_HTML[];

float readAverageEncoderDistanceM(long &leftTicksOut, long &rightTicksOut);
String buildKalmanSettingsJson();
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
  json += "\"running\":" + String(autoRunActive ? "true" : "false") + ",";
  json += "\"wifiMode\":\"" + String(WiFi.getMode() == WIFI_AP ? "AP" : "STA") + "\",";
  json += "\"ip\":\"" + currentIp.toString() + "\",";
  json += "\"ssid\":\"" + currentSsid + "\",";
  json += "\"leftTicks\":" + String(leftTicks) + ",";
  json += "\"rightTicks\":" + String(rightTicks) + ",";
  json += "\"encoderDistanceM\":" + String(encoderDistance, 3) + ",";
  json += "\"targetDistanceM\":" + String(TARGET_DISTANCE_M, 3) + ",";
  json += "\"rawRollDeg\":" + String(rawMotion.rollDeg, 2) + ",";
  json += "\"rawPitchDeg\":" + String(rawMotion.pitchDeg, 2) + ",";
  json += "\"rawYawDeg\":" + String(rawMotion.yawDeg, 2) + ",";
  json += "\"rawVelocityMps\":" + String(rawMotion.velocityMps, 3) + ",";
  json += "\"rawDistanceM\":" + String(rawMotion.distanceM, 3) + ",";
  json += "\"rawAccelMps2\":" + String(rawMotion.accelMagnitudeMps2, 3) + ",";
  json += "\"kalmanRollDeg\":" + String(kalmanMotion.rollDeg, 2) + ",";
  json += "\"kalmanPitchDeg\":" + String(kalmanMotion.pitchDeg, 2) + ",";
  json += "\"kalmanYawDeg\":" + String(kalmanMotion.yawDeg, 2) + ",";
  json += "\"kalmanVelocityMps\":" + String(kalmanMotion.velocityMps, 3) + ",";
  json += "\"kalmanDistanceM\":" + String(kalmanMotion.distanceM, 3) + ",";
  json += "\"kalmanAccelMps2\":" + String(kalmanMotion.accelMagnitudeMps2, 3) + ",";
  json += "\"motionTimeMs\":" + String(autoRunActive ? (millis() - motionStartMillis) : 0) + ",";
  json += "\"qAngle\":" + String(kalmanSettings.qAngle, 4) + ",";
  json += "\"qBias\":" + String(kalmanSettings.qBias, 4) + ",";
  json += "\"rMeasure\":" + String(kalmanSettings.rMeasure, 4) + ",";
  json += "\"rollOffsetDeg\":" + String(kalmanSettings.rollOffsetDeg, 3) + ",";
  json += "\"pitchOffsetDeg\":" + String(kalmanSettings.pitchOffsetDeg, 3) + ",";
  json += "\"stillAccelThreshold\":" + String(kalmanSettings.stillAccelThreshold, 3) + ",";
  json += "\"stillVelocityThreshold\":" + String(kalmanSettings.stillVelocityThreshold, 3) + ",";
  json += "\"velocityDecay\":" + String(kalmanSettings.velocityDecay, 3);
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

void handleKalmanSettingsGet() {
  server.send(200, "application/json", buildKalmanSettingsJson());
}

void handleKalmanSettings() {
  if (server.hasArg("qAngle")) {
    kalmanSettings.qAngle = server.arg("qAngle").toFloat();
  }
  if (server.hasArg("qBias")) {
    kalmanSettings.qBias = server.arg("qBias").toFloat();
  }
  if (server.hasArg("rMeasure")) {
    kalmanSettings.rMeasure = server.arg("rMeasure").toFloat();
  }
  if (server.hasArg("rollOffsetDeg")) {
    kalmanSettings.rollOffsetDeg = server.arg("rollOffsetDeg").toFloat();
  }
  if (server.hasArg("pitchOffsetDeg")) {
    kalmanSettings.pitchOffsetDeg = server.arg("pitchOffsetDeg").toFloat();
  }
  if (server.hasArg("stillAccelThreshold")) {
    kalmanSettings.stillAccelThreshold = server.arg("stillAccelThreshold").toFloat();
  }
  if (server.hasArg("stillVelocityThreshold")) {
    kalmanSettings.stillVelocityThreshold = server.arg("stillVelocityThreshold").toFloat();
  }
  if (server.hasArg("velocityDecay")) {
    kalmanSettings.velocityDecay = server.arg("velocityDecay").toFloat();
  }

  saveKalmanSettings();
  server.send(200, "application/json", buildKalmanSettingsJson());
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
  // Re-run MPU calibration (accel + gyro biases)
  int duration = 30;
  if (server.hasArg("duration")) {
    duration = server.arg("duration").toInt();
    if (duration <= 0) duration = 30;
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
  const unsigned long timeoutMs = 30000; // 30s
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
