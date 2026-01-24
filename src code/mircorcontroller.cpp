/*****************************************************************
 * Shell Eco-Marathon Motor Control (Arduino-compatible)
 * MCU: Arduino / Teensy-class
 * Motor: 48/52V 2000W BLDC Hub (via external controller)
 * Control Mode: Speed-set via PWM (1–4.2V equivalent)
 *
 * Philosophy:
 *  - Energy efficiency over performance
 *  - Gentle ramps
 *  - Long coast phases
 *  - Joulemeter-safe operation
 *****************************************************************/

//////////////////// CONFIG ////////////////////

// PWM
constexpr int PWM_PIN  = 2;
constexpr int PWM_FREQ = 20000;     // 20 kHz
constexpr int PWM_RES  = 12;        // 12-bit (falls back safely on Arduino)

// ADC
constexpr int ADC_PIN = A0;

// Speed set voltage mapping (ADC counts)
constexpr uint16_t ADC_MIN = 820;    // ≈1.0V
constexpr uint16_t ADC_MAX = 3450;   // ≈4.2V

// Deadzone to prevent micro-power waste
constexpr float CMD_DEADZONE = 0.03f;   // <3% treated as zero

// Duty limits
constexpr float MAX_DUTY = 0.28f;

// Ramp behavior (per second)
constexpr float RAMP_UP_RATE   = 0.8f;
constexpr float RAMP_DOWN_RATE = 2.5f;   // Slightly faster coast release

// ADC filter
constexpr float ADC_ALPHA = 0.05f;

// Safety
constexpr float SIGNAL_TIMEOUT = 0.25f;  // seconds

//////////////////// STATE ////////////////////

float adcFiltered = 0.0f;
float duty        = 0.0f;
float targetDuty  = 0.0f;

uint32_t lastMicros     = 0;
uint32_t lastSignalTime = 0;
bool armed = false;

//////////////////// SETUP ////////////////////

void setup() {
  analogReadResolution(12);

#if defined(TEENSYDUINO)
  analogWriteResolution(PWM_RES);
  analogWriteFrequency(PWM_PIN, PWM_FREQ);
#endif

  pinMode(PWM_PIN, OUTPUT);
  analogWrite(PWM_PIN, 0);

  lastMicros     = micros();
  lastSignalTime = micros();
}

//////////////////// LOOP ////////////////////

void loop() {
  uint32_t now = micros();
  float dt = (now - lastMicros) * 1e-6f;
  lastMicros = now;

  if (dt <= 0 || dt > 0.1f) return;

  // ---- Read & filter ADC ----
  uint16_t adc = analogRead(ADC_PIN);

  if (adc < ADC_MIN || adc > ADC_MAX) {
    // Signal out of bounds → decay safely
    targetDuty = 0.0f;
  } else {
    adcFiltered += ADC_ALPHA * (adc - adcFiltered);
    lastSignalTime = now;
  }

  // ---- Normalize command ----
  float cmd = (adcFiltered - ADC_MIN) / (ADC_MAX - ADC_MIN);
  cmd = constrain(cmd, 0.0f, 1.0f);

  // ---- Deadzone enforcement ----
  if (cmd < CMD_DEADZONE)
    cmd = 0.0f;

  // ---- Arming logic (ESC-like, but safe) ----
  if (!armed) {
    if (cmd == 0.0f)
      armed = true;
    else
      cmd = 0.0f;
  }

  targetDuty = cmd * MAX_DUTY;

  // ---- Signal timeout ----
  if ((now - lastSignalTime) * 1e-6f > SIGNAL_TIMEOUT) {
    targetDuty = 0.0f;
  }

  // ---- Soft ramp ----
  float maxUp   = RAMP_UP_RATE   * dt;
  float maxDown = RAMP_DOWN_RATE * dt;

  if (duty < targetDuty)
    duty += min(targetDuty - duty, maxUp);
  else
    duty -= min(duty - targetDuty, maxDown);

  duty = constrain(duty, 0.0f, MAX_DUTY);

  // ---- Output PWM ----
#if defined(TEENSYDUINO)
  uint16_t pwm = duty * ((1 << PWM_RES) - 1);
#else
  uint8_t pwm = duty * 255;   // Arduino fallback
#endif

  analogWrite(PWM_PIN, pwm);
}

