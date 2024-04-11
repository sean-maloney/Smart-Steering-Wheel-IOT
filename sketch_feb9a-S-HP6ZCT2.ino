#include <WiFi.h>
#include "secrets.h"
#include <WiFiClient.h>
#include <Adafruit_MMA8451.h>
#include <TinyGPSPlus.h>
#include <Adafruit_Sensor.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <DFRobot_DHT11.h>
#include "MAX30100_PulseOximeter.h"
#include "ThingSpeak.h"

DFRobot_DHT11 DHT;

const int trigPin_1 = 15;
const int echoPin_1 = 13;

const int trigPin_2 = 5;
const int echoPin_2 = 4;



#define REPORTING_PERIOD_MS 1000
#define DHT11_PIN 18
String s = "www.google.com/maps/dir/";

PulseOximeter pox;
float tsLastReport = 0;
float BPM = -1.0F;
int bpm = 0, temp = 0, yaxis = 0;

Adafruit_MMA8451 mma = Adafruit_MMA8451();

char ssid[] = SECRET_SSID;  // your network SSID (name)
char pass[] = SECRET_PASS;  // your network password
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
int getTemp() {
  DHT.read(DHT11_PIN);
  Serial.print("temp: ")
  Serial.print(DHT.temperature);
  Serial.print(" humi:");
  Serial.println(DHT.humidity);
  Serial.println("\n");
  delay(1000);
  temp= tempC;
  return temp;
  /*int adcVal = analogRead(PIN_LM35);
  float milliVolt = adcVal * (ADC_VREF_mV / ADC_RESOLUTION);
  float tempC = milliVolt / 10;
  Serial.print("Temperature: ");
  Serial.print(tempC);  // print the temperature in °C
  Serial.print("°C\n");
  temp = tempC;
  return temp;
  //Lm35 is working in it's Non linear range within the ESP32*/
}
void setup() {
  Serial.begin(115200);

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

  while (!Serial) {
    ;  // wait for serial port to connect. Needed for Leonardo native USB port only
  }


  WiFi.mode(WIFI_STA);
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


  /* Serial.println("Initializing pulse oximeter..");

  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;)
      ;
  } else {
    Serial.println("SUCCESS");
  }

  pox.setOnBeatDetectedCallback(onBeatDetected);*/
}

/*void onBeatDetected() {
  Serial.println("Beat!");
}*/

void loop() {
  // pox.update();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected.");
  }
  yaxis = getAcc();
  temp = getTemp();
  bpm = getBPM();

  ThingSpeak.setField(1, yaxis);
  ThingSpeak.setField(2, temp);
  ThingSpeak.setField(3, gbpm);
  //ThingSpeak.setField(4, humi);

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
  if (SW == 1) {

  } else if (SW == 0) {
    bpm = 0;
    yaxis = 0;
    temp = 900;  //average temperture of a car on fire
  }
}

static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (SerialGPS.available())
      gps.encode(SerialGPS.read());
  } while (millis() - start < ms);
}

int getBPM() {
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    Serial.print("Heart rate:");
    Serial.print(pox.getHeartRate());
    tsLastReport = millis();
  }
  return int(80);
}

int getAcc() {
  sensors_event_t event;
  mma.getEvent(&event);
  Serial.print(event.acceleration.y);
  yaxis = event.acceleration.y;
  return int(yaxis);
}

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
