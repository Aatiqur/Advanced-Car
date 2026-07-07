#include <Arduino.h>
#include <EEPROM.h>

#ifndef DISTANCE_SHARED_TYPES
#define DISTANCE_SHARED_TYPES 1
struct KalmanAxisState {
	float x[2];
	float P[2][2];
};

struct KalmanSettings {
	uint32_t magic;
	float qAngle;
	float qBias;
	float rMeasure;
	float rollOffsetDeg;
	float pitchOffsetDeg;
	float stillAccelThreshold;
	float stillVelocityThreshold;
	float velocityDecay;
	float gyroStillThreshold;
};

struct MotionState {
	float rollDeg;
	float pitchDeg;
	float yawDeg;
	float velocityMps;
	float distanceM;
	float accelMagnitudeMps2;
};
#endif

extern KalmanSettings kalmanSettings;
extern MotionState rawMotion;
extern MotionState kalmanMotion;
extern KalmanAxisState rollKalmanState;
extern KalmanAxisState pitchKalmanState;
extern float accelXBiasG;
extern float gyroXBiasDps;
extern float gyroYBiasDps;
extern float gyroZBiasDps;
extern unsigned long lastImuMicros;
extern const float GRAVITY;
extern const float ACCEL_SCALE;
extern const float GYRO_SCALE;
extern const int EEPROM_SIZE;
extern const uint32_t EEPROM_MAGIC;

void resetKalmanAxis(KalmanAxisState &state) {
	state.x[0] = 0.0f;
	state.x[1] = 0.0f;
	state.P[0][0] = 1.0f;
	state.P[0][1] = 0.0f;
	state.P[1][0] = 0.0f;
	state.P[1][1] = 1.0f;
}

void loadKalmanSettings() {
	EEPROM.begin(EEPROM_SIZE);
	EEPROM.get(0, kalmanSettings);

	if (kalmanSettings.magic != EEPROM_MAGIC || isnan(kalmanSettings.qAngle) || isnan(kalmanSettings.qBias) || isnan(kalmanSettings.rMeasure)) {
		kalmanSettings.magic = EEPROM_MAGIC;
		kalmanSettings.qAngle = 0.01f;
		kalmanSettings.qBias = 0.003f;
		kalmanSettings.rMeasure = 0.7f;
		kalmanSettings.rollOffsetDeg = 0.0f;
		kalmanSettings.pitchOffsetDeg = 0.0f;
		kalmanSettings.stillAccelThreshold = 0.15f;
		kalmanSettings.stillVelocityThreshold = 0.02f;
		kalmanSettings.velocityDecay = 0.90f;
		kalmanSettings.gyroStillThreshold = 1.0f;
		EEPROM.put(0, kalmanSettings);
		EEPROM.commit();
	}
}

void saveKalmanSettings() {
	kalmanSettings.magic = EEPROM_MAGIC;
	EEPROM.put(0, kalmanSettings);
	EEPROM.commit();
}

String buildKalmanSettingsJson() {
	String json = "{";
	json += "\"qAngle\":" + String(kalmanSettings.qAngle, 4) + ",";
	json += "\"qBias\":" + String(kalmanSettings.qBias, 4) + ",";
	json += "\"rMeasure\":" + String(kalmanSettings.rMeasure, 4) + ",";
	json += "\"rollOffsetDeg\":" + String(kalmanSettings.rollOffsetDeg, 3) + ",";
	json += "\"pitchOffsetDeg\":" + String(kalmanSettings.pitchOffsetDeg, 3) + ",";
	json += "\"stillAccelThreshold\":" + String(kalmanSettings.stillAccelThreshold, 4) + ",";
	json += "\"stillVelocityThreshold\":" + String(kalmanSettings.stillVelocityThreshold, 4) + ",";
	json += "\"velocityDecay\":" + String(kalmanSettings.velocityDecay, 4);
	json += ",\"gyroStillThreshold\":" + String(kalmanSettings.gyroStillThreshold, 4);
	json += "}";
	return json;
}

float kalmanUpdateAxis(KalmanAxisState &state, float accAngle, float gyroRate, float dt) {
	float A00 = 1.0f;
	float A01 = -dt;
	float A10 = 0.0f;
	float A11 = 1.0f;

	float x0 = A00 * state.x[0] + A01 * state.x[1] + dt * gyroRate;
	float x1 = A10 * state.x[0] + A11 * state.x[1];
	state.x[0] = x0;
	state.x[1] = x1;

	float P00 = A00 * state.P[0][0] + A01 * state.P[1][0];
	float P01 = A00 * state.P[0][1] + A01 * state.P[1][1];
	float P10 = A10 * state.P[0][0] + A11 * state.P[1][0];
	float P11 = A10 * state.P[0][1] + A11 * state.P[1][1];

	float Ppred00 = P00 * A00 + P01 * A01 + kalmanSettings.qAngle;
	float Ppred01 = P00 * A10 + P01 * A11;
	float Ppred10 = P10 * A00 + P11 * A01;
	float Ppred11 = P10 * A10 + P11 * A11 + kalmanSettings.qBias;

	state.P[0][0] = Ppred00;
	state.P[0][1] = Ppred01;
	state.P[1][0] = Ppred10;
	state.P[1][1] = Ppred11;

	float innovation = accAngle - state.x[0];
	float s = state.P[0][0] + kalmanSettings.rMeasure;
	float k0 = state.P[0][0] / s;
	float k1 = state.P[1][0] / s;

	state.x[0] += k0 * innovation;
	state.x[1] += k1 * innovation;

	float P00Old = state.P[0][0];
	float P01Old = state.P[0][1];

	state.P[0][0] -= k0 * P00Old;
	state.P[0][1] -= k0 * P01Old;
	state.P[1][0] -= k1 * P00Old;
	state.P[1][1] -= k1 * P01Old;

	return state.x[0];
}

void computeGravityCompensatedAccel(float axG, float ayG, float azG, float rollDeg, float pitchDeg, float &linX, float &linY, float &linZ) {
	float rollRad = rollDeg * PI / 180.0f;
	float pitchRad = pitchDeg * PI / 180.0f;

	float gravityX = -sinf(pitchRad);
	float gravityY = sinf(rollRad) * cosf(pitchRad);
	float gravityZ = cosf(rollRad) * cosf(pitchRad);

	linX = (axG - gravityX) * GRAVITY;
	linY = (ayG - gravityY) * GRAVITY;
	linZ = (azG - gravityZ) * GRAVITY;
}

float vectorMagnitude(float x, float y, float z) {
	return sqrt((x * x) + (y * y) + (z * z));
}

void integrateMotion(MotionState &state, float accelMagnitudeMps2, float dt) {
	if (accelMagnitudeMps2 < kalmanSettings.stillAccelThreshold) {
		accelMagnitudeMps2 = 0.0f;
		if (fabs(state.velocityMps) < kalmanSettings.stillVelocityThreshold) {
			state.velocityMps = 0.0f;
		} else {
			state.velocityMps *= kalmanSettings.velocityDecay;
		}
	} else {
		state.velocityMps += accelMagnitudeMps2 * dt;
	}

	if (fabs(state.velocityMps) < 0.01f) {
		state.velocityMps = 0.0f;
	}

	state.accelMagnitudeMps2 = accelMagnitudeMps2;
	state.distanceM += state.velocityMps * dt;
}

void resetMotionEstimates() {
	rawMotion = {};
	kalmanMotion = {};
	resetKalmanAxis(rollKalmanState);
	resetKalmanAxis(pitchKalmanState);
	lastImuMicros = micros();
}
