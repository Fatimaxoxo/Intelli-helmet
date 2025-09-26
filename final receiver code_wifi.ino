#include <WiFi.h>
#include <WiFiServer.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>


const char* ssid = "MIST";
const char* password = "Il0veMIST";

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


#define LED_PIN 12
#define BUZZER_PIN 4


WiFiServer server(80);

struct AlertData {
  String event;
  String soldier_id;
  String latitude;
  String longitude;
  String heart_rate;
  String spo2;
  bool active;
  unsigned long timestamp;
};

AlertData currentAlert;
bool alertDisplayed = false;
bool panicActive = false;
unsigned long lastBlinkTime = 0;
bool ledState = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(LED_PIN, HIGH);
  tone(BUZZER_PIN, 1000, 500);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  noTone(BUZZER_PIN);

  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed - continuing without display");
  } else {
    displayStatus("Starting...");
  }

  connectToWiFi();

  server.begin();
  Serial.println("HTTP server started");
  Serial.print("Receiver ready at: http://");
  Serial.println(WiFi.localIP());

  displayStatus("Ready - Waiting\nfor alerts...");
  currentAlert.active = false;
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    handleClient(client);
  }

  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  if (!panicActive) {
    updateDisplay();
    handleLEDBlink(); // Handle LED blinking in main loop
    if (currentAlert.active && (millis() - currentAlert.timestamp > 30000)) {
      clearAlert();
    }
  }

  delay(100);
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  displayStatus("Connecting WiFi...");
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
    Serial.print("Receiver IP address: ");
    Serial.println(WiFi.localIP());
    displayStatus("WiFi Connected\nIP: " + WiFi.localIP().toString());
    delay(3000);
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
  } else {
    Serial.println("WiFi connection failed!");
    displayStatus("WiFi Failed!");
  }
}

void handleClient(WiFiClient client) {
  String request = client.readStringUntil('\r');
  Serial.println("Request: " + request);
  client.flush();
  String response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
  if (request.indexOf("GET /alert") >= 0) {
    handleFallAlert(request);
    response += "Fall alert received successfully!";
  }
  else if (request.indexOf("GET /panic") >= 0) {
    handlePanic(request);
    response += "Panic signal received!";
  } else {
    response += "<html><body>Receiver Ready</body></html>";
  }
  client.print(response);
  client.stop();
}

void handleFallAlert(String request) {
  Serial.println("FALL ALERT RECEIVED!");
  currentAlert.event = "FALL_DETECTED";
  currentAlert.soldier_id = extractParameter(request, "soldier=");
  currentAlert.latitude = extractParameter(request, "lat=");
  currentAlert.longitude = extractParameter(request, "lon=");
  currentAlert.heart_rate = extractParameter(request, "hr=");
  currentAlert.spo2 = extractParameter(request, "spo2=");
  currentAlert.active = true;
  currentAlert.timestamp = millis();
  alertDisplayed = false;
  panicActive = false;

 
  soundAlarm();
  printSMSMessage();
}

void handlePanic(String request) {
  Serial.println("PANIC SIGNAL RECEIVED!");
  clearAlert();
  panicActive = true;

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.setTextColor(SSD1306_WHITE);
  display.println("PANIC");
  display.println("MODE");
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.println("Data Cleared");
  display.println("System Locked");
  display.display();

  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 3000, 200);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
    delay(200);
  }

  Serial.println("Panic mode displayed on OLED!");
}

String extractParameter(String request, String param) {
  int startIndex = request.indexOf(param);
  if (startIndex == -1) return "N/A";
  startIndex += param.length();
  int endIndex = request.indexOf('&', startIndex);
  if (endIndex == -1) endIndex = request.indexOf(' ', startIndex);
  if (endIndex == -1) endIndex = request.length();
  return request.substring(startIndex, endIndex);
}

void soundAlarm() {
  Serial.println("Sounding alarm...");
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 2000, 300);
    delay(300);
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
    delay(200);
  }
}

void handleLEDBlink() {
  if (currentAlert.active && !panicActive) {
    // Blink LED every 500ms when alert is active
    if (millis() - lastBlinkTime >= 500) {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
      lastBlinkTime = millis();
    }
  }
}

void printSMSMessage() {
  Serial.println(" SMS TO ADMIN ");
  Serial.println("FALL DETECTED");
  Serial.println("Soldier: " + currentAlert.soldier_id);
  Serial.println("Heart Rate: " + currentAlert.heart_rate + " bpm");
  Serial.println("SpO2: " + currentAlert.spo2 + "%");
  Serial.println("Location: " + currentAlert.latitude + ", " + currentAlert.longitude);
  Serial.println("===========================");
}

void updateDisplay() {
  if (currentAlert.active && !alertDisplayed) {
    displayAlert();
    alertDisplayed = true;
  }
}

void displayAlert() {
  Serial.println("Updating OLED with fall alert...");
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.setTextColor(SSD1306_WHITE);
  display.println("FALL");
  display.println("DETECTED");
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.println("ID: " + currentAlert.soldier_id);
  display.println("HR:" + currentAlert.heart_rate + " SpO2:" + currentAlert.spo2);
  String latStr = currentAlert.latitude;
  String lonStr = currentAlert.longitude;
  if (latStr.length() > 8) latStr = latStr.substring(0, 8);
  if (lonStr.length() > 8) lonStr = lonStr.substring(0, 8);
  display.println("Lat: " + latStr);
  display.println("Lon: " + lonStr);
  display.display();
}

void displayStatus(String message) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Military Alert System");
  display.println("Receiver Unit");
  display.println("");
  display.println(message);
  display.display();
}

void clearAlert() {
  currentAlert.active = false;
  alertDisplayed = false;
  panicActive = false;
  digitalWrite(LED_PIN, LOW);
  ledState = false;
  displayStatus("Ready - Waiting\nfor alerts...");
  Serial.println("Alert cleared - ready for new alerts");
}