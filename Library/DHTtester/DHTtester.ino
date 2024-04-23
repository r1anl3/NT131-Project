#include "DHT.h"

#define DHTPIN 4 // SDA/D2
#define DHTTYPE DHT22   // DHT 11

DHT dht(DHTPIN, DHTTYPE);
char buf1[20];

void setup() {
  Serial.begin(9600);
  Serial.println(F("DHTxx test!"));
  
  dht.begin();
}

void loop() {
  // Wait a few seconds between measurements.
  delay(2000);

  // Read humid as Percentage
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  // float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)){
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  // float hif = dht.computeHeatIndex(f, h);
  // // Compute heat index in Celsius (isFahreheit = false)
  // float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.println(F("°C "));
  // Serial.println(f);
  // Serial.print(F("°F  Heat index: "));
  // Serial.print(hic);
  // Serial.print(F("°C "));
  // Serial.print(hif);
  // Serial.println(F("°F"));
}
