#include <DHT.h>
#include <HP20x_dev.h>
#include "Adafruit_VEML7700.h"

#define DHTPIN 2
#define DHTTYPE DHT22
#define PinAir A0

Adafruit_VEML7700 veml = Adafruit_VEML7700();
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600); // Commence la communication série à 9600 bauds
  dht.begin();
  HP20x.begin();
  veml.begin();
  pinMode(PinAir, INPUT);
}

void loop() {
  // Lecture des données des capteurs
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  long pressure = HP20x.ReadPressure() / 100.0; // Convertit en hPa
  int airQuality = analogRead(PinAir);
  float lux = veml.readLux();

  // Envoie des données au récepteur
  Serial.print("T");
  Serial.println(temperature);
  Serial.print("H");
  Serial.println(humidity);
  Serial.print("P");
  Serial.println(pressure);
  Serial.print("Q");
  Serial.println(airQuality);
  Serial.print("L");
  Serial.println(lux);

  delay(1000); // Attend une seconde avant de lire à nouveau les capteurs
}
