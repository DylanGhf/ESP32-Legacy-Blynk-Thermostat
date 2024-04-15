// Setting up BLYNK server 

#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "user16"
#define BLYNK_TEMPLATE_NAME "user16@server.wyns.it"
// Setting up libraries for Sensor and LCD display for local display

#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(23, 22, 21, 25, 32, 33);
#define ONE_WIRE_BUS 12
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
int pin = 18;
int led = 19;

// Setting up stepper library to control our thermostat and attaching pins
#include <Arduino.h>
#include <Stepper.h>
#define steps 2038

Stepper stepper(steps, 18, 2 ,5 ,4);
int pos = 0;

// Libraries so our ESP32 can connect to internet.
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
// Setting up global variables our program is gonna use.
float sliderValue;
float temperature;
float manualTemperature;
float oldManualTemp;
float oldDigitalTemp;
float setTemp;
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "gtOKkOGI-03LtU7hd2JryA5-h_6BkSZU";
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "xxxx";
char pass[] = "xxxx";

// This function allows us to pull data from our sensor and writes it to the variable and onto the Blynk app3
// it also reads the set temperature on the blynk app and manual mode.

void sensor1() {
  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);
  Blynk.virtualWrite(V3, temperature);
  manualTemperature = map(analogRead(35), 0, 4096, 0, 31);
  Blynk.virtualWrite(V4, manualTemperature);
}
BLYNK_WRITE(V5) {
  sliderValue = param.asInt();
  Serial.print("Slider value: ");
  Serial.println(sliderValue);
}

// this  function detects if there was any change in our set temperature.
// this makes it so the transation between manual mode and online mode goes smoothly and automaticly.
// It basicly reads if there has been a new temperature set. If there has been, it updates the old temperature set to the new one
void update() {
  if (oldManualTemp != manualTemperature) {
    oldManualTemp = manualTemperature;
    setTemp = manualTemperature;
  }
  if (oldDigitalTemp != sliderValue) {
    oldDigitalTemp = sliderValue;
    setTemp = sliderValue;
  }
}


void setup() {
// Initializing some pin out and inputs
// also initialized our display that we are gonna use. In this case a 16x2 LCD.

  pinMode(pin, OUTPUT);
  pinMode(pin, HIGH);
  pinMode(led, OUTPUT);
  pinMode(ONE_WIRE_BUS, INPUT);
  lcd.begin(16,2);


  Serial.begin(115200);
  delay(10);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  int wifi_ctr = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Blynk.begin(auth, ssid, pass, "server.wyns.it", 8081);
}

void loop() {
// Here is where most of the magic happens, The functions we defined above are now being put to use in a loop.
// We sample sensor data and input temperatures and then update local display and online display.
  Blynk.run();
  sensor1();
  update();


  
  delay(100);

  lcd.setCursor(0,0);
  lcd.print(temperature);
  lcd.setCursor(6,0);
  lcd.print("Cur T");
  lcd.setCursor(0,1);
  lcd.print(setTemp);
  lcd.setCursor(6,1);
  lcd.print("Set T");

// Here we write even more data to the blynk app so u can also see wether the thermostat has been turned on or if it is still off.
// With remember 'pos' reffering to position, we prevent the steppermotor from turning constantly with each adjustment as it now remembers what state it is in.
// Another big plus with this system is that not every manual thermostat is the same. By changing Stepper.step(x) we can adjust for different sized thermostat to still be able to work with this attach on device
// Making this a very 'general' product applicable to a lot of older thermostats. 
// Ofcouse one could just attach the esp32 directly to the heating element and send an on signal, but where would the fun in that be? 


  Blynk.virtualWrite(V2, setTemp);
  if (temperature < setTemp) {
    digitalWrite(led, HIGH);
    Blynk.virtualWrite(V1, "Thermostat ON");
    if (pos != 0){
      pos = 0;
      stepper.setSpeed(10);
      stepper.step(-1000);
      
    }
  }
  if (temperature > setTemp) {
    digitalWrite(led, LOW);
    Blynk.virtualWrite(V1, "Thermostat OFF");
    if (pos != 1){
      pos = 1;
      stepper.setSpeed(10);
      stepper.step(2000);
     
    }
  }
  // For debugging purposes i left this part of the code in so that we can debug our program if needs be via the serial monitor if hooked up to a computer.
  Serial.println(setTemp);
}
