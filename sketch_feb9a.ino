#include <Adafruit_Sensor.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include "ThingSpeak.h"
#include <TinyGPSPlus.h>
#include <Wire.h>
#include <Arduino.h>
#include "DHT_Async.h"
#include "MAX30100_PulseOximeter.h"
#include "secrets.h"
#include "homepage.h"
//#include <Adafruit_MMA8451.h>
;

const int trigPin1 = 15;
const int echoPin1 = 13;

const int trigPin2 = 5;
const int echoPin2 = 4;

#define SW 7

long duration1;
int distance1;

long duration2;
int distance2;

WebServer server(80);

#define REPORTING_PERIOD_MS 1000

#define DHT_SENSOR_TYPE DHT_TYPE_11
static const int DHT_SENSOR_PIN = 10;
DHT_Async dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
float temperature;
float humidity;

String s = "www.google.com/maps/dir/";

PulseOximeter pox;
float tsLastReport = 0;
float BPM = -1.0F;
int bpm = 0, yaxis = 0, distance_1 = 0, distance_2 = 0;

//Adafruit_MMA8451 mma = Adafruit_MMA8451();
const char* ssid = "Seansiphone";
const char* password = "gabtoocool";
//char ssid[] = SECRET_SSID;  // your network SSID (name)
//har pass[] = SECRET_PASS;  // your network password
int keyIndex = 0;           // your network key Index number (needed only for WEP)
WiFiClient client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char *myWriteAPIKey = SECRET_WRITE_APIKEY;

String myStatus = "";

unsigned long interval = 10000;
static const uint32_t GPSBaud = 9600;
unsigned long previousMillis = 0;
int data_counter;

const size_t BUFSIZE = 300;
char f_buffer[BUFSIZE];
float *f_buf = (float *)f_buffer;

TinyGPSPlus gps;
HardwareSerial SerialGPS(2);


//temp function to simulate temp sensor
String getTemp() {
return String(temperature);
Serial.print(temperature);
Serial.print("\n");
}

String getHumi() {
return String(humidity);
Serial.print(humidity);
Serial.print("\n");
}
void setup() {
  Serial.begin(115200);
  /*
  Serial.println("Adafruit MMA8451 test!");


  if (!mma.begin()) {
    Serial.println("Couldnt start");
    while (1)
      ;
  }
  Serial.println("MMA8451 found!");

  mma.setRange(MMA8451_RANGE_2_G);

  Serial.print("Range = ");
  Serial.print(2 << mma.getRange());
  Serial.println("G");
*/
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for Leonardo native USB port only
  }


  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  ThingSpeak.begin(client);  // Initialize ThingSpeak

  SerialGPS.begin(GPSBaud);

  Serial.println("Starting...");
  SerialGPS.println("\r");
  SerialGPS.println("AT\r");
  delay(10);

  SerialGPS.println("\r");
  SerialGPS.println("AT+GPS=1\r");

  delay(100);
  SerialGPS.println("AT+CREG=2\r");
  delay(6000);

  //SerialGPS.print("AT+CREG?\r");
  SerialGPS.println("AT+CGATT=1\r");
  delay(6000);

  SerialGPS.println("AT+CGDCONT=1,\"IP\",\"WWW\"\r");
  delay(6000);

  // SerialGPS.println("AT+LOCATION=1\r");
  SerialGPS.println("AT+CGACT=1,1\r");
  delay(6000);

  //Initialize ends
  //Initialize GPS
  SerialGPS.println("\r");
  SerialGPS.println("AT+GPS=1\r");
  delay(1000);

  //SerialGPS.println("AT+GPSMD=1\r");   // Change to only GPS mode from GPS+BDS, set to 2 to revert to default.
  SerialGPS.println("AT+GPSRD=10\r");
  delay(100);

  // set SMS mode to text mode
  SerialGPS.println("AT+CMGF=1\r");
  // delay(1000);

  //SerialGPS.println("AT+LOCATION=2\r");

  Serial.println("Setup Executed");
  // send test text message
  SerialGPS.print("AT+CMGS=\"+353876770543\"\r");  //Replace this with your mobile number
  delay(1000);
  SerialGPS.print("Hi from A9G");
  SerialGPS.write(0x1A);
  delay(1000);

  server.on("/", handleRoot);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}


void handleRoot() {
  String message = homePagePart1 + getTemp() + homePagePart2 + getHumi() + homePagePart3 + /*getAcc()*/ 50 + homePagePart4 + getBPM() + homePagePart5;  // + getO2() + homePagePart6 ;
  server.send(200, "text/html", message);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}


void loop() {
  // pox.update();
  server.handleClient();
  //yaxis = getAcc();
  if (measure_environment(&temperature, &humidity)) {
        Serial.print("T = ");
        Serial.print(temperature, 1);
        Serial.print(" deg. C, H = ");
        Serial.print(humidity, 1);
        Serial.println("%");
    }
  bpm = getBPM();
  distance_1 = mirror1();
  distance_2 = mirror2();

  ThingSpeak.setField(1, 80);
  ThingSpeak.setField(2, temperature);
  ThingSpeak.setField(3, bpm);
  ThingSpeak.setField(4, humidity);
  ThingSpeak.setField(5, distance_1);
  ThingSpeak.setField(6, distance_2);

  ThingSpeak.setStatus(myStatus);

  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200) {
    Serial.println("Channel update successful.");
  } else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  smartDelay(2000);

  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));

  unsigned long currentMillis = millis();

  if ((unsigned long)(currentMillis - previousMillis) >= interval) {

    send_gps_data();
    previousMillis = currentMillis;
  }
    
  delay(20000);
}

int Emergency() {  //simulates a bad crash
  if (digitalRead(SW) == HIGH) {

  } else if (digitalRead(SW) == LOW) {
    bpm = 0;
    //yaxis = 0;
    temperature = 900;  //average temperture of a car on fire
  }
}

static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (SerialGPS.available())
      gps.encode(SerialGPS.read());
  } while (millis() - start < ms);
}

void ultraSonic(int trig, int echo, long &duration, int &distance) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  duration = pulseIn(echo, HIGH);
  distance = duration * 0.0343 / 2;
}

float mirror1() {
  ultraSonic(trigPin1, echoPin1, duration1, distance1);
  // Print the distance to the Serial Monitor
  Serial.print("\nDistance 1: ");
  Serial.print(distance1);
  Serial.println(" cm");
  distance_1 = distance1;
  return distance_1;
}
float checkMirror1() {
  if (distance_1 > 30) {
    Serial.print("Right OK, Clear to merge");
  } else {
    Serial.print("Right UnSafe, DO NOT MERGE");
  }
}

float mirror2() {
  ultraSonic(trigPin2, echoPin2, duration2, distance2);
  // Print the distance to the Serial Monitor
  Serial.print("\nDistance 2: ");
  Serial.print(distance2);
  Serial.println(" cm");
  distance_2 = distance2;
  return distance_2;
}

String checkMirror2() {
  if (distance_2 > 30) {
    Serial.print("Right OK, Clear to merge");
  } else {
    Serial.print("Right UnSafe, DO NOT MERGE");
  }
}

int getBPM() {
  /*if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    Serial.print("Heart rate:");
    Serial.print(pox.getHeartRate());
    tsLastReport = millis();
  }*/
  return int(80);  //returning false value but avearge human heart rate
  /*HEart rate refuses to work due to the thing speak delay, will constanly now return a 
  average heartrate of 80 BPM to simulate a live human*/
}
/*
int getAcc() {
  sensors_event_t event;
  mma.getEvent(&event);
  Serial.print(event.acceleration.y);
  yaxis = event.acceleration.y;
  return int(yaxis);
}
*/


void send_gps_data() {
  if (gps.location.lat() == 0 || gps.location.lng() == 0) {
    Serial.println("Return Executed");
    return;
  }

  data_counter++;

  Serial.print("Latitude (deg): ");
  f_buf[data_counter] = gps.location.lat();
  Serial.println(f_buf[data_counter]);

  Serial.print("Longitude (deg): ");
  f_buf[data_counter + 1] = gps.location.lng();
  Serial.println(f_buf[data_counter + 1]);

  Serial.println(data_counter);
  Serial.println();

  s += String(gps.location.lat(), 6);
  s += ",";
  s += String(gps.location.lng(), 6);
  s += "/";

  Serial.println(s);

  if (data_counter >= 10) {
    data_counter = 0;

    Serial.println("Sending Message");

    SerialGPS.println("AT+CMGF=1\r");  //set to text format
    delay(1000);

    SerialGPS.println("AT+CNMI=2,2,0,0,0\r");
    delay(1000);

    SerialGPS.print("AT+CMGS=\"+353876770543\"\r");  //Replace this with your mobile number
    delay(1000);
    SerialGPS.print(s);
    SerialGPS.write(0x1A);
    delay(1000);
    s = "www.google.com/maps/dir/";  //reset to null location
  }
}

  static bool measure_environment(float *temperature, float *humidity) {
    static unsigned long measurement_timestamp = millis();

    /* Measure once every four seconds. */
    if (millis() - measurement_timestamp > 4000ul) {
        if (dht_sensor.measure(temperature, humidity)) {
            measurement_timestamp = millis();
            return (true);
        }
    }

    return (false);
}
