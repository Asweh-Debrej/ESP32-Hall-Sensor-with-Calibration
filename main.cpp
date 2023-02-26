#include <Arduino.h>

const int ledBlue = 21;
const int ledRed = 19;

bool redValid;
bool blueValid;

int val;

int timerSerial;
int timerHall;
const int intervalSerial = 500;
const int intervalHall = 20;

int tresRedHigh;
int tresBlueLow;
int tresRedLow;
int tresBlueHigh;

int tune;
int tuneTotal = 0;
int tuneCount = 0;

long normalize(int x, int min, int max) {
  if (x < min) {
    x = min;
  } else if (x > max) {
    x = max;
  }

  return x;
}


void tuneWarn(int red, int blue) { // Warns the user for initialize tuning. Delays all process due to using while
  int timer1 = millis();
  int timer2 = millis();
  bool redBlue;

  while (millis() < timer1 + 4000) {
    if (millis() >= timer2 + 200) {
      digitalWrite(red, redBlue);
      digitalWrite(blue, !redBlue);
      redBlue = !redBlue;
      timer2 = millis();
    }
  }

  digitalWrite(red, LOW);
  digitalWrite(blue, LOW);
}

void tuneStart(int red, int blue) {
  int timer1 = millis();
  int timer2 = millis();
  int maxCount;
  int tresMid;
  bool led;
  tuneTotal = 0;
  tuneCount = 0;

  // Warn room magnetic field tuning
  Serial.println("Reading environment, put away magnet!");
  while (millis() < timer1 + 4000) {
    if (millis() >= timer2 + 500) {
      digitalWrite(red, led);
      digitalWrite(blue, led);
      led = !led;
      timer2 = millis();
    }
  }

  digitalWrite(red, HIGH);
  digitalWrite(blue, HIGH);

  timer1 = millis();
  timer2 = millis();
  while (millis() < timer1 + 4000) {
    if (millis() >= timer2 + 100) {
      tuneTotal += hallRead();
      tuneCount++;
    }
  }

  digitalWrite(red, LOW);
  digitalWrite(blue, LOW);
  tresMid = (int)(tuneTotal/tuneCount);

  timer1 = millis();
  timer2 = millis();
  tuneTotal = 0;
  tuneCount = 0;
  maxCount = 0;

  // Warn north (+ / red) magnetic field tuning
  Serial.println("Reading north (+ / red), put north polar!");
  while (millis() < timer1 + 4000) {
    if (millis() >= timer2 + 500) {
      digitalWrite(red, led);
      timer2 = millis();
      led = !led;
    }
  }

  digitalWrite(red, HIGH);
  timer1 = millis();
  timer2 = millis();

  while (millis() < timer1 + 4000) {
    if (millis() >= timer2 + 100) {
      maxCount++;
      if (hallRead() < tresMid) {
        tuneTotal += hallRead();
        tuneCount++;
      }
    }
  }

  digitalWrite(red, LOW);
  if (tuneCount >= (4000/100) * 0.8) { // Tuning must reach 80% success rate to be valid
    redValid = true;
    tresRedLow = (int)(tresMid - (tresMid - tuneTotal/tuneCount)*0.8);
    tresRedHigh = (int)(tresMid - (tresMid - tuneTotal/tuneCount)*0.2);
  } else {
    redValid = false;
  }

  timer1 = millis();
  timer2 = millis();
  tuneTotal = 0;
  tuneCount = 0;
  maxCount = 0;

  // Warn south (- / blue) magnetic field tuning
  Serial.println("Reading south (- / blue), put south polar!");
  while (millis() < timer1 + 4000) {
    if (millis() >= timer2 + 500) {
      digitalWrite(blue, led);
      timer2 = millis();
      led = !led;
    }
  }

  digitalWrite(blue, HIGH);
  timer1 = millis();
  timer2 = millis();

  while (millis() < timer1 + 4000) {
    if (millis() >= timer2 + 100) {
      maxCount++;
      if (hallRead() > tresMid) {
        tuneTotal += hallRead();
        tuneCount++;
      }
    }
  }

  digitalWrite(blue, LOW);
  if (tuneCount >= (4000/100) * 0.8) { // Tuning must reach 80% success rate to be valid
    blueValid = true;
    tresBlueHigh = (int)(tresMid - (tresMid - tuneTotal/tuneCount)*0.8);
    tresBlueLow = (int)(tresMid - (tresMid - tuneTotal/tuneCount)*0.2);
  } else {
    blueValid = false;
  }

  Serial.println("Tuning done!");
}

void setup() {
  pinMode(ledRed, OUTPUT);
  pinMode(ledBlue, OUTPUT);
  Serial.begin(115200);
  delay(500);
  timerSerial = millis();
  timerHall = millis();
  tuneWarn(ledRed, ledBlue);
  tuneStart(ledRed, ledBlue);
}

void loop() {
  if (millis() >= timerHall + intervalHall) {
    val = hallRead();
    if (redValid) {
      if (val < tresRedHigh) {
        analogWrite(ledRed, normalize(map(hallRead(), tresRedHigh, tresRedLow, 0, 256), 0, 256));
      } else {
        analogWrite(ledRed, 0);
      }
    }

    if (blueValid) {
      if (val > tresBlueLow) {
        analogWrite(ledBlue, normalize(map(hallRead(), tresBlueLow, tresBlueHigh, 0, 256), 0, 256));
      } else {
        analogWrite(ledBlue, 0);
      }
    }

    timerHall = millis();
  }

  if (millis() >= timerSerial + intervalSerial) {
    Serial.printf("Val = %d, (%d, %d)\n", val, normalize(map(hallRead(), tresRedHigh, tresRedLow, 0, 256), 0, 256), normalize(map(hallRead(), tresBlueLow, tresBlueHigh, 0, 256), 0, 256));
    timerSerial = millis();
  }
}
