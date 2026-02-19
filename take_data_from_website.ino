
/*
 *  This sketch sends data via HTTP GET requests to $host.
 *
 *  Magic numbers are used to determine which line(s) to handle, and which part of this line is used.
 *  The numbers are determined using curl (or some other http dump program)
 */
#if defined(ESP8266)
#pragma message "ESP8266 stuff happening!"
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#pragma message "ESP32 stuff happening!"
#include <WiFi.h>
#else
#error "This ain't a ESP8266 or ESP32.. NO WIFI FOR YOU!"
#endif

#include <WiFiClientSecure.h>

//lcd libraries
#include <Wire.h>
#include "rgb_lcd.h"
#include <math.h>

#define LIGHT_SENSOR 34//Grove - Light Sensor is connected to ___ of esp32

const char* ssid     = "John Ballums Konservatorservice";
const char* password = "ballumkonservdk";
//NEW URL: https://vejret.stenomuseet.dk/data/stenomuseet.htm
const char* host = "moonwrecker.neocities.org";
// This is the path to the page we want on the server:
String url = "/projects/IOT";

const int thresholdvalue=10;         //The treshold for which the LED should turn on. Setting it lower will make it go on at more light, higher for more darkness
float Rsensor; //Resistance of sensor in K  

//lcd color
TwoWire _theWire = TwoWire(0);
rgb_lcd lcd;
const int colorR = 0;
const int colorG = 255;
const int colorB = 0;

void setup() {
  _theWire.begin(21, 22, 100000);
  Serial.begin(115200);
  randomSeed(analogRead(0));
  delay(10);
  // Initialize LCD 
  lcd.begin(16, 2,LCD_5x8DOTS,_theWire);
  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

int value = 0;
int pcounter = 0;


void loop() {
  int pEnd = 0;

  int sensorValue = analogRead(LIGHT_SENSOR); 
  Rsensor = (float)(1023-sensorValue)*10/sensorValue;

  int linecounter=0;
  String slurString;
  float slur;
  String line="";
  ++value;

  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  lcd.setRGB(0,0,0);
  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClientSecure client;
  client.setInsecure();
  const int httpPort = 443;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  // The header should look like this:
  /*
  
  */
  client.print(
              String("GET ") + url + " HTTP/1.1\r\n" +  //Server + protocol
               "Host: " + host + "\r\n" + // Host
               "Connection: close\r\n" +  // I don't want to talk after you sent me the data.
               "\r\n"                     // Blank line at end of header.
              );
  
  delay(100); //wait a while to let the server respond (increase on shitty connections)
  pEnd = random(8);
  // Read all the lines of the reply from server
  while(client.connected() & sensorValue>200){
    line = client.readStringUntil('\r'); //cariage return as delimiter

    Serial.println(line); //print out every received line.
    
    if (line.startsWith("p",2) & !(line.startsWith("U",1))) {  //fetch paragraphs only
      pcounter = pcounter + 1;
      int lineStart = line.indexOf('>')+1; //after the > in <p>

      int lineEnd = line.lastIndexOf('<'); //before the < in </p>
      Serial.println(pcounter);
      Serial.println(pEnd);
      if (pcounter == pEnd) {
        slurString = line.substring(lineStart,lineEnd); //string becomes text in paragraph
        Serial.println(slurString); //print string
      }
    }

    linecounter++; //count up for every line we read.
    
    yield(); //Avoid a crash by letting the ESP core handle its wifi stuff.
  }




  if(line!="" & sensorValue>200) //if data exists
  {
    Serial.println(slurString);

    lcd.setRGB(colorR, colorG, colorB);
    if (slurString.length() < 16) {
      lcd.print(String(slurString)); //test report
    }
    else {
      lcd.setRGB(colorR, colorG, colorB);
      String shortSlur = slurString.substring(1,16);
      int newLine = shortSlur.lastIndexOf(' ') + 1;

      lcd.print(slurString.substring(0,newLine));
      lcd.setCursor(0, 1);
      lcd.print(slurString.substring(newLine+1));
      lcd.setCursor(0, 0);
    }
    pcounter = 0;
  }

  Serial.println();
  Serial.println("closing connection");
  
  delay(5*1000); //run every 30 seconds (in milliseconds)

}
