#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// opened eyes pixel pattern
byte open_eyes[8] = {
  B01110,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B01110,
  B00000
};

// closed eyes 
byte close_eyes[8] = {
  B00000,
  B00000,
  B00000,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000
};

// happy eyes
byte happy_eyes[8] = {
  B00000,
  B00000,
  B00000,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000
};

byte heart[8] = {
  B00000,
  B01010,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000,
  B00000
};

byte check_mark[8] = {
  B00000,
  B00001,
  B00011,
  B10110,
  B11100,
  B01000,
  B00000,
  B00000
};

byte bell[8] = {
  B00100,
  B01110,
  B01110,
  B01110,
  B11111,
  B00000,
  B00100,
  B00000
};

byte snowflake[8] = {
  B00100,
  B10101,
  B01110,
  B11111,
  B01110,
  B10101,
  B00100,
  B00000
};

int leftEyePos = 6;
int rightEyePos = 9;

unsigned long lastBlink = 0;
unsigned long blinkDuration = 200;      // how long eyes stay closed (ms)
unsigned long blinkInterval = 3000;     // how often is blinked
bool eyesClosed = false;

const int coldLedPin = 8;
const int warmLedPin = 9;
const int tempPin = A0;
const float tempThreshold = 20;         // °C -> when he gets cold

const int switchPin = 6;
int switchState = 0;
int prevSwitchState = 0;

unsigned long maxIdleDuration = 20000;     // how long staying idle before sleeping
unsigned long idleDuration =  0;
bool sleeping = false;

enum RobotState{
  AWAKE,
  SLEEP,
  HAPPY,
  WAKING_UP,
  COLD,
  ANGRY,
  SAD,
  TIRED
};

RobotState currentState = AWAKE;

unsigned long stateEnteredAt = 0;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  pinMode(switchPin, INPUT);
  pinMode(coldLedPin, OUTPUT);
  pinMode(warmLedPin, OUTPUT);

  lcd.createChar(0, open_eyes);
  lcd.createChar(1, close_eyes);
  lcd.createChar(2, heart);
  lcd.createChar(3, check_mark);
  lcd.createChar(4, bell);
  lcd.createChar(5, snowflake);

  showEyes(false);
}

void loop() {
  unsigned long now = millis();
  switchState = digitalRead(switchPin);
 
 switch(currentState){
  case AWAKE:
    updateAwake(now);
    break;
  case SLEEP:
    updateSleeping();
    break;
  case HAPPY:
    updateHappy(now);
    break;
  case WAKING_UP:
    updateWakingUp();
    break;
  case COLD:
    updateCold();
    break;
  case ANGRY:
  case SAD:
  case TIRED:
  break;
 }
  prevSwitchState = switchState;
}

void changeState(RobotState newState){
  currentState = newState;
  stateEnteredAt = millis();
}

void updateAwake(unsigned long now){

  float temp = readTemperature();

  if(temp< tempThreshold){
    changeState(COLD);
    return; 
  }

  // blink logic
  if (!eyesClosed && now - lastBlink >= blinkInterval) {
    // close eye
    Serial.println("Blinking");
    showEyes(true);
    eyesClosed = true;
    lastBlink = now;
  } 
  else if (eyesClosed && now - lastBlink >= blinkDuration) {
    // open eye
    Serial.println("Opening eyes");
    showEyes(false);
    eyesClosed = false;
    lastBlink = now;
  }

  if(now - stateEnteredAt >= maxIdleDuration){
    goingToSleep();
    sleep();
    
    changeState(SLEEP);
  }

  // show heart if button is touched
  if(switchState != prevSwitchState && switchState == LOW){
    showHeartEyes();
    changeState(HAPPY);
  }

}

void updateHappy(unsigned long now){

  Serial.print("Temperatur: ");
Serial.println(readTemperature());
  if(now - stateEnteredAt >= 1000){
    showEyes(false);
    changeState(AWAKE);
  }
}

void updateSleeping(){
  if(switchState != prevSwitchState && switchState == LOW){
    changeState(WAKING_UP);
  }
}

void updateWakingUp(){
  wakeUp();
  showEyes(false);
  changeState(AWAKE);
}

void updateCold(){
  digitalWrite(coldLedPin, HIGH);
  digitalWrite(warmLedPin, LOW);

  showCold();

  float temp = readTemperature();

  if(temp >= tempThreshold){
    digitalWrite(coldLedPin, LOW);
    digitalWrite(warmLedPin, HIGH);
    changeState(AWAKE);
  }
}

float readTemperature() {
  int sensorValue = analogRead(tempPin);
  float voltage = sensorValue * (5.0 / 1023.0);
  float temperatureC = (voltage - 0.5) * 100.0;
  return temperatureC;
}


// 'ANIMATIONS'
void showEyes(bool closed) {
  lcd.clear();
  lcd.setCursor(leftEyePos, 0);
  lcd.write(closed ? byte(1) : byte(0));
  lcd.setCursor(rightEyePos, 0);
  lcd.write(closed ? byte(1) : byte(0));
}

void showHeartEyes(){
  // eyes
  lcd.clear();
  lcd.setCursor(leftEyePos, 0);
  lcd.write("^");
  lcd.setCursor(rightEyePos, 0);
  lcd.write("^");

  // show other hearts on screen
  lcd.setCursor(2, 0);
  lcd.write(byte(2));
  lcd.setCursor(1, 1);
  lcd.write(byte(2));
  lcd.setCursor(15, 0);
  lcd.write(byte(2));
  lcd.setCursor(10, 1);
  lcd.write(byte(2));
}

void goingToSleep(){

  lcd.setCursor(leftEyePos, 0);
  lcd.write("-");
  lcd.setCursor(leftEyePos, 0);
  lcd.write("_");
  delay(400);  
  lcd.setCursor(rightEyePos, 0);
  lcd.write("-");
  lcd.setCursor(rightEyePos, 0);
  lcd.write("_");
  delay(400);  
}

void sleep(){
  lcd.clear();
  // closed eyes 
  lcd.setCursor(leftEyePos, 1);
  lcd.write(byte(1));
  lcd.setCursor(rightEyePos, 1);
  lcd.write(byte(1));

  // render 'zzz'
  lcd.setCursor(11, 0);
  lcd.write("Z");
  lcd.setCursor(12, 0);
  lcd.write("z");
  lcd.setCursor(13, 0);
  lcd.write("z");

}

void wakeUp(){
  Serial.println("Waking up");
  
  lcd.setCursor(leftEyePos, 1);
  lcd.write("o");
  lcd.setCursor(rightEyePos, 1);
  lcd.write(byte(1));
  lcd.setCursor(11, 0);
  lcd.write("Z");
  lcd.setCursor(12, 0);
  lcd.write("z");
  lcd.setCursor(13, 0);
  lcd.write("z");
  delay(400);  

  lcd.setCursor(11, 0);
  lcd.write(" ");
  delay(400);

  lcd.setCursor(12, 0);
  lcd.write(" ");
  delay(400);

  lcd.setCursor(13, 0);
  lcd.write(" ");
  delay(400);
}

void showCold(){
  lcd.clear();
  lcd.setCursor(leftEyePos, 0);
  lcd.write(">");
  lcd.setCursor(rightEyePos, 0);
  lcd.write("<");

  lcd.setCursor(2, 0);
  lcd.write(byte(5));

  lcd.setCursor(4, 0);
  lcd.write(byte(5));

  lcd.setCursor(11, 0);
  lcd.write(byte(5));

  lcd.setCursor(14, 0);
  lcd.write(byte(5));

  lcd.setCursor(16, 0);
  lcd.write(byte(5));

   lcd.setCursor(0, 1);
  lcd.write(byte(5));

  lcd.setCursor(3, 1);
  lcd.write(byte(5));

   lcd.setCursor(5, 1);
  lcd.write(byte(5));

  lcd.setCursor(11, 1);
  lcd.write(byte(5));

  lcd.setCursor(15, 1);
  lcd.write(byte(5));

}

