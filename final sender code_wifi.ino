#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>


const char* ssid = "MIST";
const char* password = "Il0veMIST";


const char* receiverIP = "10.103.135.96";
const int receiverPort = 80;

#define PANIC_PIN 33
#define LED_PIN 2
#define BUZZER_PIN 4


bool systemActive = true;
bool fallDetected = false;
float latitude = 23.8103;
float longitude = 90.4125;
int heartRate = 75;
int spO2 = 98;
unsigned long lastFallCheck = 0;

WiFiClient client;

void setup() {
  Serial.begin(115200);
  pinMode(PANIC_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);


  digitalWrite(LED_PIN, HIGH);
  tone(BUZZER_PIN, 1000, 500);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  noTone(BUZZER_PIN);

  connectToWiFi();

  Serial.println("Simple Sender Unit Ready!");
  Serial.println("Press panic button or wait for simulated fall detection");
}

void loop() {

  if (digitalRead(PANIC_PIN) == LOW) {
    activatePanicMode();
    return;
  }

  if (!systemActive) return;


  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }


  if (millis() - lastFallCheck > 30000) {
    Serial.println("SIMULATED FALL DETECTED!");
    fallDetected = true;
    lastFallCheck = millis();

  
    heartRate = random(60, 100);
    spO2 = random(95, 100);
    latitude = 23.8103 + (random(-100, 100) / 10000.0);
    longitude = 90.4125 + (random(-100, 100) / 10000.0);
  }

  
  if (fallDetected) {
    sendFallAlert();
    fallDetected = false;
    delay(5000);
  }

  delay(100);
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("Sender IP address: ");
    Serial.println(WiFi.localIP());
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
  } else {
    Serial.println("WiFi connection failed!");
  }
}

void sendFallAlert() {
  Serial.println("Sending fall alert to receiver...");

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No WiFi connection!");
    showLocalAlert();
    return;
  }

  if (client.connect(receiverIP, receiverPort)) {
    String request = "GET /alert?";
    request += "event=fall_detected";
    request += "&soldier=SOLDIER_001";
    request += "&lat=" + String(latitude, 6);
    request += "&lon=" + String(longitude, 6);
    request += "&hr=" + String(heartRate);
    request += "&spo2=" + String(spO2);
    request += " HTTP/1.1\r\n";
    request += "Host: " + String(receiverIP) + "\r\n";
    request += "Connection: close\r\n\r\n";

    client.print(request);

    // Wait for response
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println("Client timeout!");
        client.stop();
        showLocalAlert();
        return;
      }
    }
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    client.stop();
    Serial.println();
    Serial.println("Fall alert sent successfully!");

    // LED + buzzer pattern
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, HIGH);
      tone(BUZZER_PIN, 1500, 300);
      delay(300);
      digitalWrite(LED_PIN, LOW);
      noTone(BUZZER_PIN);
      delay(300);
    }

  } else {
    Serial.println("Connection to receiver failed!");
    showLocalAlert();
  }
}

void showLocalAlert() {
  Serial.println("FALL ALERT ");
  Serial.println("FALL DETECTED!");
  Serial.println("Soldier: SOLDIER_001");
  Serial.println("Location: " + String(latitude, 6) + ", " + String(longitude, 6));
  Serial.println("Heart Rate: " + String(heartRate) + " bpm");
  Serial.println("SpO2: " + String(spO2) + "%");
  Serial.println("========================");


  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 2000, 200);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
    delay(200);
  }
}

void activatePanicMode() {
  Serial.println("PANIC MODE ACTIVATED!");
  systemActive = false;

  latitude = 0;
  longitude = 0;
  heartRate = 0;
  spO2 = 0;

  sendPanicSignal();

  for (int i = 0; i < 15; i++) {
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 3000, 100);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
    delay(100);
  }

  Serial.println("System permanently deactivated!");
  while (true) {
    delay(1000);
  }
}

void sendPanicSignal() {
  Serial.println("Sending panic signal...");
  if (client.connect(receiverIP, receiverPort)) {
    String request = "GET /panic?";
    request += "event=panic_activated";
    request += "&soldier=SOLDIER_001 HTTP/1.1\r\n";
    request += "Host: " + String(receiverIP) + "\r\n";
    request += "Connection: close\r\n\r\n";

    client.print(request);
    client.stop();
    Serial.println("Panic signal sent!");
  } else {
    Serial.println(" PANIC ALERT ");
    Serial.println("PANIC BUTTON PRESSED");
    Serial.println("ALL DATA CLEARED");
    Serial.println("===================");
  }
}