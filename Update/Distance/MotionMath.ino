// MotionMath.ino — math helpers used by MPUMotion.ino
void computeGravityCompensatedAccel(float axG, float ayG, float azG,
                                   float rollDeg, float pitchDeg,
                                   float &linX, float &linY, float &linZ) {
  float rollRad  = rollDeg  * PI / 180.0f;
  float pitchRad = pitchDeg * PI / 180.0f;

  // Expected gravity unit vector in the body frame given the current tilt
  float gravityX = -sinf(pitchRad);
  float gravityY =  sinf(rollRad) * cosf(pitchRad);
  float gravityZ =  cosf(rollRad) * cosf(pitchRad);

  // Subtract gravity, scale g -> m/s^2
  linX = (axG - gravityX) * GRAVITY;
  linY = (ayG - gravityY) * GRAVITY;
  linZ = (azG - gravityZ) * GRAVITY;
}

float vectorMagnitude(float x, float y, float z) {
  return sqrt(x * x + y * y + z * z);
}

// Naive trapezoidal integration of accel magnitude into velocity + distance.
// Zeros velocity when the robot is essentially still so drift doesn't accumulate.
const float STILL_ACCEL_THRESHOLD = 0.25f;   // m/s^2
const float STILL_VELOCITY_THRESHOLD = 0.03f; // m/s
const float VELOCITY_DECAY = 0.85f;          // multiplicative decay when below threshold

void integrateMotion(MotionState &state, float accelMagMps2, float dt) {
  if (accelMagMps2 < STILL_ACCEL_THRESHOLD) {
    if (fabs(state.velocityMps) < STILL_VELOCITY_THRESHOLD) {
      state.velocityMps = 0.0f;
    } else {
      state.velocityMps *= VELOCITY_DECAY;
    }
  } else {
    state.velocityMps += accelMagMps2 * dt;
  }

  if (fabs(state.velocityMps) < 0.01f) {
    state.velocityMps = 0.0f;
  }

  state.accelMagnitudeMps2 = accelMagMps2;
  state.distanceM += state.velocityMps * dt;
}

void resetMotionEstimates() {
  float savedRollOffset  = motion.rollOffsetDeg;
  float savedPitchOffset = motion.pitchOffsetDeg;
  motion = {};
  // Keep roll/pitch offsets — they describe the physical mounting, not the run
  motion.rollOffsetDeg  = savedRollOffset;
  motion.pitchOffsetDeg = savedPitchOffset;
  lastImuMicros = micros();
}
