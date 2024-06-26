#define BLYNK_PRINT Serial
/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN "YOUR_AUTH_TOKEN"

#include <NTPClient.h>
// #include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "DHT.h"
// #include <BlynkSimpleEsp8266.h>
#include "MQ135.h"
// #include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "foo.h"

#define DHTPIN 4 // SDA/D2
#define DHTTYPE DHT11   // DHT 11
#define PIN_MQ135 A0 // MQ135

BlynkTimer timer;
WiFiUDP ntpUDP;
DHT dht(DHTPIN, DHTTYPE);
MQ135 mq135_sensor = MQ135(PIN_MQ135);
float humid, temp, hic; // Values from DHT11
float correctedRZero, resistance, correctedPPM, rzero, ppm; // Values from MQ135 
const char *ssid     = "YOUR_SSID"; // Wifi name
const char *password = "YOUR_PASSWORD";// Wifi password

const char* serverName = "YOUR_SERVER";
String sensorLocation = "Station_01";
String apiKeyValue = "YOUR_SERVER_API_KEY";
String sensorName = "DHT11-MQ135";

NTPClient timeClient(ntpUDP, "vn.pool.ntp.org", 25200, 60000); // ntpUDP, "VN", UTC7(in second), time update interval
char weekDay [7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"}; // Weekday format
String realTime; // Time format
String sensorError;
String location;
long delayTime = 1000;
long lastPost;
int httpResponseCode;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // 9600 baud
  WiFi.begin(ssid, password); // Connect to wifi

  while ( WiFi.status() != WL_CONNECTED ) { // Waiting for wifi connection
    delay ( 500 );
    Serial.print ( "." );
  }
  Serial.println("");

  //Start components
  timeClient.begin(); // NTPTimeClient start (1 cái server thời gian)
  dht.begin();// DHT11 start
  // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password); // Connect Blynk server

  timer.setInterval(delayTime, sendDataToBlynk); // Delay time for sending information to Blynk server
}

void loop() {
  // put your main code here, to run repeatedly:
  long currTime = millis();
  helloWorld();
  getData(); // lấy dữ liệu từ sensor
  // sendDataToBlynk(); // gửi lên blynk
  printData(); // in dữ liệu ra serial
  // if (currTime - lastPost > 30000) {
  //   sendDataToLAMP();
  //   lastPost = currTime;
  // } // mỗi 30s post lên server 1 lần
  delay(delayTime);
}

String getLocation() {
  String curr;
  // Your code here to get location

  return curr;
}

void getData() {
  // Get data from components
  sensorError = "None";

  timeClient.update(); // Update time
  Blynk.run();// Run Blynk

  humid = dht.readHumidity(); // Read humid
  temp = dht.readTemperature(); // Read temperature as Celsius (the default)
  correctedRZero = mq135_sensor.getCorrectedRZero(temp, humid); // Calculate RZero value
  correctedPPM = mq135_sensor.getCorrectedPPM(temp, humid); // Calcualte PPM value
  resistance = mq135_sensor.getResistance(); // Resistance
  hic = dht.computeHeatIndex(temp, humid, false); // Compute heat index in Celsius (isFahreheit = false)
  location = getLocation();
  realTime = String(weekDay[timeClient.getDay()]) + String(' ') + timeClient.getFullFormattedTime() + String('|') + location + String('|'); // Formated datetime
  rzero = mq135_sensor.getRZero();
  ppm = mq135_sensor.getPPM();

  if (isnan(humid) || isnan(temp)) { // Check if any reads failed and exit early (to try again).
    humid = 0;
    temp = 0;
    correctedPPM = 0;
    sensorError = "Failed to read from DHT11 sensor!";
    Serial.println(sensorError);
    return;
  }

  if (correctedPPM < 1) {
    sensorError = "Failed to read from MQ135 sensor!";
    Serial.println(sensorError);
    return;
  }
}

void printData() {
  Serial.print(realTime);
  Serial.print(F("Humidity: "));
  Serial.print(humid);
  Serial.print(F("%  Temperature: "));
  Serial.print(temp);
  Serial.print(F("°C "));
  Serial.print(F("Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print("Corrected RZero: ");
  Serial.print(correctedRZero);
  Serial.print(" Resistance: ");
  Serial.print(resistance);
  Serial.print(" Corrected PPM: ");
  Serial.print(correctedPPM);
  Serial.println("ppm");
}

void sendDataToBlynk() {
  // Send data to Blynk
  Blynk.virtualWrite(V0, temp); // Virtual pin 0, tempurature
  Blynk.virtualWrite(V1, humid); // Virtual pin 1, humid
  Blynk.virtualWrite(V2, realTime); // Virual pin 2, datetime
  Blynk.virtualWrite(V3, correctedPPM); // Virtual pin 3, ppm
  if (sensorError == "None") {
    Blynk.virtualWrite(V4, httpResponseCode);   
  } else {
    Blynk.virtualWrite(V4, sensorError); // Virtual pin 4, status
  }

  if (temp > 34 || humid < 80) {
    Blynk.logEvent("dry", "Get yourself some water!"); // tên sự kiện, thông tin
  }
  if (correctedPPM > 42) {
    Blynk.logEvent("polution", "Please wearing mash outside!");
  }
  if (httpResponseCode != 200) {
    Blynk.logEvent("post", "Code: " + httpResponseCode);
  }
}

void sendDataToLAMP() {
  // Send data to LAMP
    if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;
    WiFiClient client;
    
    // Your Domain name with URL path or IP address with path
    http.begin(client, serverName);
    
    // Specify content-type header
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    // Prepare your HTTP POST request data
    String httpRequestData = "api_key=" + apiKeyValue + "&sensor=" + sensorName
                          + "&location=" + sensorLocation + "&value1=" + String(temp)
                          + "&value2=" + String(humid) + "&value3=" + String(correctedPPM) + "";
    Serial.println("");
    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);
    
    // Send HTTP POST request
    httpResponseCode = http.POST(httpRequestData);
        
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode); // -1
    }
    // Free resources
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }
  Serial.println("");
  //Send an HTTP POST request every 30 seconds
}