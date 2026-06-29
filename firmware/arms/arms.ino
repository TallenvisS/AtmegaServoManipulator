#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <EEPROM.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

// ===== KONFIGURACJA =====
#define SERVO_COUNT 5
#define MAX_PRESETS 8
#define MAX_SEQ 10
#define DEFAULT_SPEED 1000 

// ===== KALIBRACJA SERW =====
int servoMin[SERVO_COUNT] = {197, 169, 155, 141, 238};
int servoMax[SERVO_COUNT] = {461, 550, 544, 544, 600};

// ===== KĄTY POCZĄTKOWE =====
int startAngle[SERVO_COUNT] = {90, 90, 90, 90, 90};
int homeAngle[SERVO_COUNT]  = {90, 180, 90, 180, 180};

// ===== ZMIENNE RUCHU =====
int currentPos[SERVO_COUNT];    
int startPos[SERVO_COUNT];      
int targetPos[SERVO_COUNT];     
unsigned long moveStartTime = 0;
unsigned long moveDuration = DEFAULT_SPEED;

int presets[MAX_PRESETS][SERVO_COUNT];
int sequence[MAX_SEQ];
int seqLen = 0, seqIndex = 0;

bool moving = false;
bool loopEnabled = false;

// ===== POMOCNICZE =====
int presetAddr(int preset, int servo) {
  return (preset * SERVO_COUNT + servo) * sizeof(int);
}

int angleToPWM(int servo, int angle) {
  angle = constrain(angle, 0, 180);
  return map(angle, 0, 180, servoMin[servo], servoMax[servo]);
}

// ===== KINEMATYKA ODWROTNA (IK) =====
void moveToPos(double x, double y, double z_floor) {
  if (x<10 && z_floor <10) {
    Serial.println("male x");
    x=10;
  }
  
  const double L1 = 11.0;
  const double L2 = 13.0;
  const double offsetZ = 10.5;

  double z = z_floor - offsetZ;
  double l = sqrt(x * x + y * y);
  double h = sqrt(l * l + z * z);

  if (h > (L1 + L2)) h = L1 + L2;
  if (h < abs(L1 - L2)) h = abs(L1 - L2);

  // Obliczanie kątów
  double b = atan2(y, x) * (180.0 / PI);
  double a0 = 90.0 + b;
  
  double phi = atan2(z, l) * (180.0 / PI);
  double cos_theta = (L1 * L1 + h * h - L2 * L2) / (2.0 * L1 * h);
  double theta = acos(constrain(cos_theta, -1.0, 1.0)) * (180.0 / PI);
  double a1 = constrain(phi + theta, 0, 180);

  double cos_beta = (L1 * L1 + L2 * L2 - h * h) / (2.0 * L1 * L2);
  double beta = acos(constrain(cos_beta, -1.0, 1.0)) * (180.0 / PI);
  double a2 = constrain(180.0 - beta, 0, 180);
  double orientacja_nadgarstka = 90.0; // Twoje "zero" nadgarstka (poziom)
  double a3 = orientacja_nadgarstka - a1 + a2;
  Serial.println(a3);
  a3 = constrain(a3, 0, 180);
  // Ustawienie celów
  prepareMove(0, 180 - (int)a0);
  prepareMove(1, (int)a1);
  prepareMove(2, (int)a2);
  prepareMove(3, (int)a3);
  startNewMove(800); // Ruch IK potrwa 0.8 sekundy
}

// ===== ZARZĄDZANIE RUCHEM SYNCHRONICZNYM =====
void prepareMove(int servo, int angle) {
  targetPos[servo] = angleToPWM(servo, angle);
}

void startNewMove(int duration) {
  for (int i = 0; i < SERVO_COUNT; i++) {
    startPos[i] = currentPos[i];
  }
  moveStartTime = millis();
  moveDuration = duration;
  moving = true;
}

void updateServos() {
  if (!moving) return;

  unsigned long elapsed = millis() - moveStartTime;
  float progress = (float)elapsed / (float)moveDuration;

  if (progress >= 1.0) {
    progress = 1.0;
    moving = false;
  }

  for (int i = 0; i < SERVO_COUNT; i++) {
    currentPos[i] = startPos[i] + (int)((targetPos[i] - startPos[i]) * progress);
    pwm.setPWM(i, 0, currentPos[i]);
  }

  if (!moving && loopEnabled) {
    seqIndex = (seqIndex + 1) % seqLen;
    loadPreset(sequence[seqIndex]);
  }
}

// ===== PRESETY =====
void savePreset(int n) {
  if (n < 0 || n >= MAX_PRESETS) return;
  for (int i = 0; i < SERVO_COUNT; i++) {
    presets[n][i] = currentPos[i];
    EEPROM.put(presetAddr(n, i), currentPos[i]);
  }
  Serial.println(F("Preset zapisany"));
}

void loadPreset(int n) {
  if (n < 0 || n >= MAX_PRESETS) return;
  for (int i = 0; i < SERVO_COUNT; i++) targetPos[i] = presets[n][i];
  startNewMove(DEFAULT_SPEED);
}

// ===== KOMENDY SERIAL =====
void handleSerial() {
  if (!Serial.available()) return;
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  if (cmd.length() == 5 && isDigit(cmd[0])) {
    int id = cmd.substring(0, 2).toInt();
    int angle = cmd.substring(2, 5).toInt();
    if (id < SERVO_COUNT) {
      prepareMove(id, angle);
      startNewMove(500);
    }
  } 
  else if (cmd.startsWith("k ")) {
    int c1 = cmd.indexOf(',');
    int c2 = cmd.indexOf(',', c1 + 1);
    double x = cmd.substring(2, c1).toDouble();
    double y = cmd.substring(c1 + 1, c2).toDouble();
    double z = cmd.substring(c2 + 1).toDouble();
    moveToPos(x, y, z);
  }
  else if (cmd.startsWith("save")) savePreset(cmd.substring(5).toInt());
  else if (cmd.startsWith("goto")) loadPreset(cmd.substring(5).toInt());
  else if (cmd.startsWith("off")) {
    int id = cmd.substring(4).toInt(); // Pobiera numer serwa po spacji
    if (id < SERVO_COUNT) {
      moving = false; // Zatrzymujemy ewentualny ruch płynny
      pwm.setPWM(id, 0, 4096); // Specjalna wartość w PCA9685, która wyłącza sygnał całkowicie
      Serial.print(F("Servo ")); Serial.print(id); Serial.println(F(" wylaczone (brak trzymania)"));
    }
  }
  else if (cmd.startsWith("o")) { 
    // 1. Zsynchronizuj cele z obecną pozycją, aby ramię stało w miejscu
    for(int i=0; i<SERVO_COUNT; i++) {
        targetPos[i] = currentPos[i]; 
    }
    // 2. Ustaw nowy cel TYLKO dla chwytaka
    targetPos[4] = angleToPWM(4, 180); 
    
    startNewMove(400); 
    Serial.println(F("Chwytak: OTWIERANIE"));
  }
  else if (cmd.startsWith("z")) {
    for(int i=0; i<SERVO_COUNT; i++) {
        targetPos[i] = currentPos[i]; 
    }
    targetPos[4] = angleToPWM(4, 0); 
    
    startNewMove(400); 
    Serial.println(F("Chwytak: ZAMYKANIE"));
  }
  else if (cmd == "stop") { moving = false; loopEnabled = false; }
  else if (cmd == "cfg") {
    for(int i=0; i<SERVO_COUNT; i++) {
      Serial.print(i); Serial.print(F(": ")); Serial.println(currentPos[i]);
    }
  }
}

// ===== GŁÓWNE FUNKCJE ARDUINO =====
void setup() {
  Serial.begin(9600);
  pwm.begin();
  pwm.setPWMFreq(50);

  for (int i = 0; i < SERVO_COUNT; i++) {
    currentPos[i] = angleToPWM(i, homeAngle[i]);
    pwm.setPWM(i, 0, currentPos[i]);
    for (int p = 0; p < MAX_PRESETS; p++) EEPROM.get(presetAddr(p, i), presets[p][i]);
  }
  Serial.println(F("Robot Ready. Uzyj: k x,y,z"));
}

void loop() {
  handleSerial();
  updateServos();
}

//Kod pisany z pomocą LLM firmy OpenAI
