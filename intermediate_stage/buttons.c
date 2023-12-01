#include <Arduino.h>
#define INT0 2
#define INT1 3

volatile byte state0 = LOW;
volatile byte state1 = LOW;
volatile byte postscaler = LOW;

unsigned long lastInterruptTime0 = 0;
unsigned long lastInterruptTime1 = 0;
const unsigned long debounceDelay = 50; // Délai de rebond en millisecondes

ISR(INT0_vect) {
  if (millis() - lastInterruptTime0 > debounceDelay) {
    state0 = !state0;
    Serial.println("pressed0.");
    lastInterruptTime0 = millis();
  }
}

ISR(INT1_vect) {
  if (millis() - lastInterruptTime1 > debounceDelay) {
    state1 = !state1;
    Serial.println("pressed1.");
    lastInterruptTime1 = millis();
  }
}

ISR(TIMER2_OVF_vect) {
  TCNT2 = 131;
  if (postscaler < 125) postscaler++;
  else {
    postscaler = 0;
    Serial.println("Hello");
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(INT0, INPUT_PULLUP);
  pinMode(INT1, INPUT_PULLUP);

  TCCR2A &= 0b11111100;
  TCCR2B &= 0b11110111;
  TCCR2B |= 0b00000111;
  TIMSK2 |= 0b000000001;

  EICRA |= 0b00000100; // Any logical change on TN0 generates an interrupt request.
  EICRA |= 0b00000001; // Any logical change on INT1 generates an interrupt request.
  EIMSK |= 0b00000001; // Configure INT1
  EIMSK |= 0b00000010; // Configure INT0
  
  interrupts();
}
