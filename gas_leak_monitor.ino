/* ============================================================
   PROJECT      : Smart Gas Leak & Kitchen Safety Monitor
   COURSE       : IoT and Robotics
   PLATFORM     : ESP32 + Wokwi Simulation + Blynk Cloud

   ALARM TRIGGER CONDITIONS:
   ┌─────────────────────────────────────────────────────────────┐
   │  GAS LEAK    Gas    > 1500 (danger) → Siren + Valve CLOSE  │
   │  OVERHEAT    Temp   > 45 °C         → Siren + Valve CLOSE  │
   │  VENTILATE   Gas    > 800 (caution) → Fan ON, no siren     │
   │  ALL CLEAR   No alarm active        → GREEN LED (silent)   │
   │  ALARM RESET V5 button (Blynk)      → Clear + Reopen valve │
   └─────────────────────────────────────────────────────────────┘
   ============================================================ */

// ---------- BLYNK CREDENTIALS ----------
#define BLYNK_TEMPLATE_ID "TMPL6o_6_4awS"
#define BLYNK_TEMPLATE_NAME "Gas Leak Safety Monitor"
#define BLYNK_AUTH_TOKEN "tmij095QrWLXB32axrJtUE_sar1NINnF"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- WIFI ----------
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ---------- PIN DEFINITIONS ----------
#define GAS_PIN       34   // MQ-2 analog output (ADC1 - safe to use with WiFi)
#define DHT_PIN       4
#define SERVO_PIN     13
#define BUZZER_PIN    25
#define RED_PIN       26
#define GREEN_PIN     27
#define BLUE_PIN      14
#define RELAY_PIN     32   // exhaust fan

#define DHT_TYPE      DHT22
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

// ---------- THRESHOLDS ----------
const int   GAS_DANGER_THRESHOLD  = 1500;  // MQ-2 raw ADC — full alarm
const int   GAS_CAUTION_THRESHOLD = 800;   // MQ-2 raw ADC — auto-ventilate only
const float FIRE_TEMP_THRESHOLD   = 45.0;  // °C — stove overheat

// ---------- SIREN TONE SETTINGS ----------
const int   SIREN_FREQ_LOW   = 900;
const int   SIREN_FREQ_HIGH  = 1600;
const unsigned long SIREN_TOGGLE_MS = 350;

// ---------- OBJECTS ----------
DHT dht(DHT_PIN, DHT_TYPE);
Servo gasValveServo;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------- STATE ----------
bool valveClosed          = false;   // false = gas flows normally (valve open)
bool manualFanOn          = false;
bool alarmActive          = false;
bool sirenHighTone        = false;
unsigned long lastSensorRead   = 0;
unsigned long lastSirenToggle  = 0;
const unsigned long SENSOR_INTERVAL = 2000;

// ============================================================
//  HELPERS
// ============================================================

void setRGB(bool r, bool g, bool b) {
  digitalWrite(RED_PIN,   r ? HIGH : LOW);
  digitalWrite(GREEN_PIN, g ? HIGH : LOW);
  digitalWrite(BLUE_PIN,  b ? HIGH : LOW);
}

// ---- Continuous alternating siren — call every loop tick ----
void runSiren() {
  if (!alarmActive) return;
  if (millis() - lastSirenToggle >= SIREN_TOGGLE_MS) {
    lastSirenToggle = millis();
    sirenHighTone   = !sirenHighTone;
    tone(BUZZER_PIN, sirenHighTone ? SIREN_FREQ_HIGH : SIREN_FREQ_LOW);
  }
}

void closeValve() {
  gasValveServo.write(0);     // 0 deg = shut off
  valveClosed = true;
  Blynk.virtualWrite(V3, 1);
  Serial.println("  >> VALVE  : CLOSED (safety shutoff)");
}

void openValve() {
  gasValveServo.write(90);    // 90 deg = flowing normally
  valveClosed = false;
  Blynk.virtualWrite(V3, 0);
  Serial.println("  >> VALVE  : OPEN (normal)");
}

void setFan(bool on) {
  digitalWrite(RELAY_PIN, on ? HIGH : LOW);
}

void triggerAlarm(String reason) {
  if (!alarmActive) {
    alarmActive     = true;
    sirenHighTone   = false;
    lastSirenToggle = 0;          // fire immediately on next runSiren()
    Serial.println("  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    Serial.println("  !! ALARM TRIGGERED: " + reason);
    Serial.println("  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    Blynk.logEvent("gas_alert", reason);
  }
  setRGB(true, false, false);     // RGB → RED
  closeValve();
  setFan(true);                   // fan on full to clear the air
}

void clearAlarm() {
  alarmActive = false;
  noTone(BUZZER_PIN);
  setRGB(false, true, false);     // RGB → GREEN
  Serial.println("  >> ALARM CLEARED. Status: SAFE");
}

// ============================================================
//  BLYNK VIRTUAL PIN HANDLERS
// ============================================================

// V3 = Valve switch (1 = closed, 0 = open) — rejected while alarm is active
BLYNK_WRITE(V3) {
  int v = param.asInt();
  if (!alarmActive) { if (v == 1) closeValve(); else openValve(); }
  else Blynk.virtualWrite(V3, 1);   // snap UI back to closed
}

// V4 = Exhaust fan manual override
BLYNK_WRITE(V4) {
  manualFanOn = param.asInt();
  setFan(manualFanOn);
  Serial.println(manualFanOn ? "  >> FAN    : MANUAL ON" : "  >> FAN    : MANUAL OFF");
}

// V5 = Alarm reset button
BLYNK_WRITE(V5) {
  if (param.asInt() == 1) { clearAlarm(); openValve(); }
}

// ============================================================
//  SETUP
// ============================================================
void setup() {

  Serial.begin(115200);
  delay(3000);
  Serial.println("============================================");
  Serial.println("  Project   : Gas Leak & Kitchen Safety Monitor");
  Serial.println("  Danger    : " + String(GAS_DANGER_THRESHOLD)  + " (gas ADC)");
  Serial.println("  Caution   : " + String(GAS_CAUTION_THRESHOLD) + " (gas ADC)");
  Serial.println("  HOW TO TEST in Wokwi:");
  Serial.println("    1. Click MQ-2  -> raise 'ppm' slider above 800/1500");
  Serial.println("    2. Click DHT22 -> raise temp above 45 C");
  Serial.println("    3. Tap V5 in Blynk app to reset alarm");
  Serial.println("============================================");

  pinMode(RED_PIN,    OUTPUT);
  pinMode(GREEN_PIN,  OUTPUT);
  pinMode(BLUE_PIN,   OUTPUT);
  pinMode(RELAY_PIN,  OUTPUT);
  pinMode(GAS_PIN,    INPUT);

  noTone(BUZZER_PIN);
  dht.begin();
  gasValveServo.attach(SERVO_PIN);
  openValve();
  setRGB(false, true, false);   // GREEN = safe
  setFan(false);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Gas Safety Monitor");
  display.println("BOOTING...");
  display.display();

  Blynk.begin(auth, ssid, pass);
  Serial.println("  System READY. Monitoring...");
}

// ============================================================
//  LOOP
// ============================================================
void loop() {
  Blynk.run();
  runSiren();      // keeps the siren alternating between sensor reads

  if (millis() - lastSensorRead >= SENSOR_INTERVAL) {
    lastSensorRead = millis();

    int   gasLevel    = analogRead(GAS_PIN);
    Serial.print("ADC = ");
    Serial.println(gasLevel);
    float temperature = dht.readTemperature();
    float humidity    = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("  [WARN] DHT22 read failed.");
      temperature = 0; humidity = 0;
    }

    // ---- Push to Blynk ----
    Blynk.virtualWrite(V0, gasLevel);
    Blynk.virtualWrite(V1, temperature);
    Blynk.virtualWrite(V2, humidity);

    // ---- THREE-TIER ALARM LOGIC ----
    if (gasLevel > GAS_DANGER_THRESHOLD) {
      triggerAlarm("GAS LEAK: " + String(gasLevel) + " > " + String(GAS_DANGER_THRESHOLD));
    }
    else if (temperature > FIRE_TEMP_THRESHOLD) {
      triggerAlarm("OVERHEAT: " + String(temperature, 1) + " C > " + String(FIRE_TEMP_THRESHOLD, 0) + " C");
    }
    else if (!alarmActive) {
      if (gasLevel > GAS_CAUTION_THRESHOLD) {
        // Caution zone: ventilate quietly, no siren, no valve shutoff
        setFan(true);
        setRGB(true, true, false);   // amber-ish (RED+GREEN) = caution
      } else {
        if (!manualFanOn) setFan(false);
        setRGB(false, true, false);  // GREEN = all clear
      }
    }

    // ---- Serial Monitor ----
    Serial.println("---------------------------------------------");
    Serial.print  ("  Gas Level : "); Serial.print(gasLevel);
    if (gasLevel > GAS_DANGER_THRESHOLD)       Serial.println("  <<< DANGER!");
    else if (gasLevel > GAS_CAUTION_THRESHOLD) Serial.println("  (caution - venting)");
    else                                        Serial.println("  [clear]");
    Serial.print  ("  Temp      : "); Serial.print(temperature, 1); Serial.print(" C");
    Serial.println(temperature > FIRE_TEMP_THRESHOLD ? "  <<< OVERHEAT!" : "");
    Serial.print  ("  Humidity  : "); Serial.print(humidity, 1); Serial.println(" %");
    Serial.print  ("  Valve     : "); Serial.println(valveClosed ? "CLOSED" : "OPEN");
    Serial.print  ("  Alarm     : "); Serial.println(alarmActive ? "!! ACTIVE !!" : "SAFE");

    // ---- OLED ----
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("STAT:");
    display.println(alarmActive ? " !! ALERT !!" : (gasLevel > GAS_CAUTION_THRESHOLD ? " VENTING" : " SAFE"));
    display.print("Gas :"); display.println(gasLevel);
    display.print("Temp:"); display.print(temperature, 1); display.println("C");
    display.print("Hum :"); display.print(humidity, 1); display.println("%");
    display.print("Valv:"); display.println(valveClosed ? "CLOSED" : "OPEN");
    display.print("Fan :"); display.println(digitalRead(RELAY_PIN) ? "ON" : "OFF");
    display.display();
  }
}
