#include "nectransmitter.h"

#define BUTTON_PIN 4
#define NAD_IR_PIN 3
#define MUSIC_PIN A0
#define LED_PIN 2
//#define DEBUG

NECTransmitter necTransmitter(NAD_IR_PIN);

void buttonPressed(){
  Serial.println("Button");
  necTransmitter.SendExtendedNEC(0x877C, 0x80);
}

void checkButtonPressed(){
  if(digitalRead(BUTTON_PIN) == LOW){
    delay(20);
    if(digitalRead(BUTTON_PIN) == LOW){
      buttonPressed();
      
      // Wait until button is released.
      while(digitalRead(BUTTON_PIN) == LOW){}
    }
  }
}

void setup() {
  pinMode(MUSIC_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

#ifdef DEBUG
  Serial.begin(115200);
  Serial.println("Starting");
#endif


  // Pulse LED a few times.
  for(int i = 0; i < 5; i++){
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      delay(100);
  }
}


// Gathers reads from the music pin for 500ms and checks if enough peaks have been found.
bool MusicDetected(){
  const auto musicDetectionStart = millis();

  unsigned int numPeaks = 0;
  while(millis() - musicDetectionStart < 500){

    // Button was soldered to a pin that doesn't support interrupts. Hack a check in here.
    checkButtonPressed();

    const auto value = analogRead(MUSIC_PIN);
    if(abs(value - 512) > 10){
        numPeaks++;
    }
  }

#ifdef DEBUG
  Serial.println("Num peaks: " + String(numPeaks));
#endif
  return numPeaks > 50;
}

// This will return immediately after a signal is detected.
void WaitForTurnOn(){
  while(!MusicDetected()){
  }
}

// This will return after the no signal was detected for a set timeout.
void WaitForTurnOff(){
    unsigned long lastMusicDetected = millis();
    while(true){
      const unsigned long currentMillis = millis();

      if(MusicDetected()){
        lastMusicDetected = currentMillis;
      }

      if(currentMillis - lastMusicDetected > 10000){
        return;
      }
    }
}

void loop() {
  static bool isOn = false;
  if(!isOn){
    WaitForTurnOn();
    isOn = true;
    digitalWrite(LED_PIN, HIGH);

#ifdef DEBUG
    Serial.println("Music started");
#endif

  } else {

    WaitForTurnOff();
    isOn = false;
    digitalWrite(LED_PIN, LOW);

#ifdef DEBUG
    Serial.println("Music stopped");
#endif
  }

  Serial.println("State change");
  necTransmitter.SendExtendedNEC(0x877C, 0x80);
}