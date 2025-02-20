#define BLYNK_TEMPLATE_ID "your_BLYNK_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "your_BLYNK_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN "your_BLYNK_AUTH_TOKEN"
#define BLYNK_PRINT Serial

#define DHTPIN D4                   // Pin where the DHT22 is connected
#define DHTTYPE DHT22               // DHT 22 sensor type

#define RELAYPINBULB D1             // Relay pin control 1
#define RELAYPINHUMIDIFIER D2       // Relay pin control 2

#define OPTIMALTEMPERATURE 37.5     // Optimal temperature
#define OPTIMALHUMIDITY 56         // If it is 56, I think no need for humidifier, it will be fine even if you add water

#define SYSTEMTEST false            // Tests the system to check if data is being sent, etc.

#define INTERVALDURATIONTEMP 10     // Check temperature every 10 seconds
#define INTERVALDURATIONSERVO 10800 // 180 minutes, which means the servo will rotate every 3 hours (3*60*60)
#define SERVOSPEED 2 

#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <Servo.h>

void LoopControlTempHumidity();
void ControlRelay(uint8_t relay, uint8_t val);
void ControlServo(int servoR, int angleR);
void ControlTempHumidity();
void ReadSensor();
void BlynkSend();
void SystemTest();

char auth[] = BLYNK_AUTH_TOKEN;  // Enter your Blynk auth token here // if it doesn't work, replace auth[] = “YOUR_BLYNK_AUTH_TOKEN”
char ssid[] = "your_ssid";   // Enter your WiFi SSID here
char pass[] = "your_password";   // Enter your WiFi password here

DHT dht(DHTPIN, DHTTYPE);
Servo servoMotor; // Servo object created

double temperature = 37.5;
int humidity = 56;
int timeEgg = 179;
int count = 0;
unsigned long currentMillisTEMP = 0;
unsigned long currentMillisSERVO = 0;
unsigned long currentMillisServoSpeed = 0;
unsigned long currentMillisTimeEgg = 0;

bool humidtyStatus = false;
bool turnServo = false;
int servoRotation = 1;
int angleRotation = 45;

void setup() {
  Serial.begin(9600);

  pinMode(RELAYPINBULB, OUTPUT);
  pinMode(RELAYPINHUMIDIFIER, OUTPUT);

  servoMotor.attach(D9); // Servo motor connected to D9 pin
  servoMotor.write(90);
  
  Blynk.begin(auth, ssid, pass);
  dht.begin();

  turnServo = true;

}

void loop() {
  Blynk.run();

  // if (isnan(temperature) || isnan(humidity)) {
  //   Serial.println("Failed to read from DHT sensor!");
  //   //return;
  // }

if(turnServo) ControlServo(servoRotation, angleRotation);

if (millis() - currentMillisSERVO >= INTERVALDURATIONSERVO) { // If 180 minutes have passed
      Serial.println("180 minutes passed!");
      currentMillisSERVO = millis(); // Reset time

      turnServo = true;
      timeEgg = 179;
  }

if (millis() - currentMillisTEMP >= INTERVALDURATIONTEMP) { // If 10 seconds have passed
      Serial.println("10 seconds passed!");
      currentMillisTEMP = millis(); // Reset time

      LoopControlTempHumidity();
      BlynkSend();
  }
  
if (millis() - currentMillisTimeEgg >= 60) { // If 1 minute has passed
      Serial.println("1 minute passed!");
      currentMillisTimeEgg = millis(); // Reset time
      timeEgg--;
      if (timeEgg < 0) timeEgg = 0;
  }

if (SYSTEMTEST) SystemTest();
}



void LoopControlTempHumidity(){
   ReadSensor(); // Get data from the sensor
   
   ControlTempHumidity(); // Turn on/off the necessary relay

   Serial.print("Temperature: ");
   Serial.print(temperature);
   Serial.print(" °C, Humidity: ");
   Serial.print(humidity);
   Serial.println(" %");
   //Serial.println("sent");
}


void ControlServo(int servoR, int angleR){
       
  if (millis() - currentMillisServoSpeed >= SERVOSPEED) { // If 2 seconds have passed
      Serial.println("2 seconds passed!");
      currentMillisServoSpeed = millis(); // Reset time

      servoMotor.write(servoMotor.read() + servoR);    // Rotate motor by 1 degree
        
    if (servoMotor.read() <= 90 - angleR || servoMotor.read() >= 90 + angleR) {
      servoR = servoR * -1;
      turnServo = false;
    }

  }
 
}
void ControlTempHumidity(){
 
  ControlTemp();
  ControlHumidity();

}

void ControlTemp(){

  if(temperature >= OPTIMALTEMPERATURE + 0.1 && digitalRead(RELAYPINBULB) == HIGH){ // If temperature is high, turn off the bulb
    ControlRelay(RELAYPINBULB, LOW);
  }
  else if(temperature <= OPTIMALTEMPERATURE - 0.1 && digitalRead(RELAYPINBULB) == LOW){ // If temperature is low, turn on the bulb
    ControlRelay(RELAYPINBULB, HIGH);
  }

}

void ControlHumidity(){

  if(humidtyStatus && humidity >= OPTIMALHUMIDITY + 2){ // If humidity is high, turn off the humidifier
    ControlRelay(RELAYPINHUMIDIFIER, HIGH); // It should be tested whether it is ON when high
    delay(400);
    ControlRelay(RELAYPINHUMIDIFIER, LOW); 
    humidtyStatus = false;

  }
  else if(!humidtyStatus && humidity <= OPTIMALHUMIDITY - 2){ // If humidity is low, turn on the humidifier
    ControlRelay(RELAYPINHUMIDIFIER, HIGH);
    delay(400); 
    ControlRelay(RELAYPINHUMIDIFIER, LOW); 
    humidtyStatus = true;
    
  }

}
void ControlRelay(uint8_t relay, uint8_t val){

  digitalWrite(relay, val); // Turn on the relay


}
void danger(){ // For example, in case of a temperature drop, what should be done if the bulb hasn't been turned on, etc.


}
void ReadSensor(){
   temperature = dht.readTemperature();
   humidity = dht.readHumidity();
}

void BlynkSend(){
  Blynk.virtualWrite(V0, timeEgg / 60);  // Send hour value to Blynk App
  Blynk.virtualWrite(V1, timeEgg % 60);     // Send minute value to Blynk App
  Blynk.virtualWrite(V5, humidity);     // Send humidity value to Blynk App
  Blynk.virtualWrite(V6, temperature);  // Send temperature value to Blynk App
 
}

void SystemTest(){
  temperature++;
  humidity++;
   timeEgg -= 15;
}
