/*****************************************************************
 * Shell Eco-Marathon Motor Control
 * MCU: Teensy 4.0
 * Motor: 48/52V 2000W BLDC Hub
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
constexpr int PWM_PIN = 2;
constexpr int PWM_FREQ = 20000;     // 20 kHz
constexpr int PWM_RES  = 12;        // 12-bit

// ADC
constexpr int ADC_PIN = A0;
constexpr int ADC_RES = 12;

// Speed set voltage mapping (ADC counts)
constexpr uint16_t ADC_MIN = 820;    // ≈1.0V
constexpr uint16_t ADC_MAX = 3450;   // ≈4.2V

// Duty limits
constexpr float MAX_DUTY = 0.28f;    // Absolute ceiling (28%)

// Ramp behavior (per second)
constexpr float RAMP_UP_RATE   = 0.8f;   // % duty per second
constexpr float RAMP_DOWN_RATE = 2.0f;   // Faster release

// ADC filter
constexpr float ADC_ALPHA = 0.05f;

//////////////////// STATE ////////////////////

float adcFiltered = 0.0f;
float duty        = 0.0f;
float targetDuty  = 0.0f;

uint32_t lastMicros = 0;

//////////////////// SETUP ////////////////////

void setup() {
  analogReadResolution(ADC_RES);
  analogWriteResolution(PWM_RES);
  analogWriteFrequency(PWM_PIN, PWM_FREQ);

  pinMode(PWM_PIN, OUTPUT);
  analogWrite(PWM_PIN, 0);

  lastMicros = micros();
}

//////////////////// LOOP ////////////////////

void loop() {
  uint32_t now = micros();
  float dt = (now - lastMicros) * 1e-6f;
  lastMicros = now;

  if (dt <= 0 || dt > 0.1f) return;

  // ---- Read & filter ADC ----
  uint16_t adc = analogRead(ADC_PIN);
  adc = constrain(adc, ADC_MIN, ADC_MAX);
  adcFiltered += ADC_ALPHA * (adc - adcFiltered);

  // ---- Normalize command ----
  float cmd = (adcFiltered - ADC_MIN) / (ADC_MAX - ADC_MIN);
  cmd = constrain(cmd, 0.0f, 1.0f);

  targetDuty = cmd * MAX_DUTY;

  // ---- Soft ramp ----
  float maxUp   = RAMP_UP_RATE   * dt;
  float maxDown = RAMP_DOWN_RATE * dt;

  if (duty < targetDuty) {
    duty += min(targetDuty - duty, maxUp);
  } else {
    duty -= min(duty - targetDuty, maxDown);
  }

  duty = constrain(duty, 0.0f, MAX_DUTY);

  // ---- Output PWM ----
  uint16_t pwm = duty * ((1 << PWM_RES) - 1);
  analogWrite(PWM_PIN, pwm);
}
