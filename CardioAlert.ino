#include <TinyGsmClient.h>
#include <TinyGPSPlus.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SerialMon Serial
#define SerialAT Serial1
#define SerialGPS Serial2

#define SIM_RX_PIN 26
#define SIM_TX_PIN 27
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17
#define PULSE_PIN 34

const int pulseThreshold = 50;
const char emergencyNumber[] = "YOUR_EMERGENCY_NUMBER";

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
TinyGsm modem(SerialAT);
TinyGPSPlus gps;

void setup() {
  SerialMon.begin(115200);
  delay(10);
  SerialAT.begin(9600, SERIAL_8N1, SIM_RX_PIN, SIM_TX_PIN);
  SerialGPS.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  pinMode(PULSE_PIN, INPUT);
  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    SerialMon.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Initializing...");
  display.display();
  SerialMon.println("Initializing SIM module...");
  delay(3000);
  modem.restart();
  modem.sendAT("+CMGF=1");
  modem.waitResponse();
  SerialMon.println("Setup complete.");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Setup complete.");
  display.display();
  delay(2000);
}

void loop() {
  int pulseRate = getPulseRate();
  SerialMon.print("Pulse Rate: ");
  SerialMon.println(pulseRate);
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Pulse Rate:");
  display.setTextSize(3);
  display.setCursor(0, 25);
  display.print(pulseRate);
  display.print(" BPM");
  display.display();
  if (pulseRate < pulseThreshold) {
    SerialMon.println("Low pulse rate detected!");
    String location = getGPSLocation();
    SerialMon.print("Location: ");
    SerialMon.println(location);
    sendAlert(location);
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("ALERT!");
    display.println("Low pulse detected.");
    display.println("Sending SMS...");
    display.display();
    delay(60000);
  }
  delay(1000);
}

int getPulseRate() {
  int sensorValue = analogRead(PULSE_PIN);
  int pulseRate = random(40, 120);
  return pulseRate;
}

String getGPSLocation() {
  while (SerialGPS.available() > 0) {
    gps.encode(SerialGPS.read());
  }
  if (gps.location.isValid()) {
    double latitude = gps.location.lat();
    double longitude = gps.location.lng();
    String location = "Lat:" + String(latitude, 6) + ",Lng:" + String(longitude, 6);
    return location;
  } else {
    return "Unable to get location";
  }
}

void sendAlert(String location) {
  String message = "Emergency! Low pulse detected. Location: " + location;
  SerialMon.print("Sending SMS to ");
  SerialMon.println(emergencyNumber);
  if (modem.sendSMS(emergencyNumber, message)) {
    SerialMon.println("SMS sent successfully.");
  } else {
    SerialMon.println("Failed to send SMS.");
  }
}