# Intelli-Helmet 

A smart wearable alert system for soldiers and security personnel.  
The Intelli-Helmet detects falls or injuries, monitors vital signs, pinpoints GPS location, and instantly transmits panic/emergency alerts to a remote base station.  

---

## 🚀 Features

- **Fall Detection (MPU6050):** Monitors acceleration and orientation to detect if a soldier has fallen or experienced impact.
- **Vital Signs (MAX30102):** Reads heart rate (BPM) and SpO₂ (%) to assess potential injury.
- **Location Tracking (NEO-6M GPS):** Acquires precise latitude and longitude coordinates.
- **Panic Button:** Allows manual triggering of an emergency alert which also clears previous data.
- **Wireless Alerts:**
  - **Wi-Fi HTTP:** Sends alert packets to a receiver ESP32 base station running a built-in web server.
  - **GSM (SIM800L):** Optional – send SMS or HTTP alerts when Wi-Fi isn’t available.
- **Base Station (Receiver):**
  - Displays soldier data and alert status on an OLED screen.
  - Activates LED and buzzer alarm patterns on incoming alerts.
  - Provides a simple web page to view alert information.

---

## 🛠️ Hardware Used

| Module / Sensor              | Purpose                  |
|----------------------------------- |----------------------------|
| ESP32 Dev Board             | Main MCU, Wi-Fi, UART    |
| MPU6050 / MPU9250    | 3-axis accelerometer & gyroscope (fall detection) |
| MAX30102 / MAX30100| Heart rate & SpO₂ sensor (injury detection) |
| NEO-6M GPS Module      | Location data (lat/lon)  |
| SIM800L GSM Module    | (Optional) SMS/HTTP over cellular |
| SSD1306 OLED (Receiver)| Display alerts           |
| Piezo Buzzer + LED           | Visual/audio alarm       |
| Tactile Panic Switch         | Manual emergency trigger |

---

## 🔗 System Overview

[ Soldier Unit / Intelli-Helmet ] --> Wi-Fi or GSM --> [ Receiver Unit / Base Station ]


- **Sender (Helmet):** ESP32 reads MPU + MAX + GPS + Panic switch → builds alert message → sends via Wi-Fi HTTP GET or GSM SMS/HTTP.
- **Receiver (Base):** ESP32 listens on port 80 → parses incoming alerts → updates OLED → triggers LED/buzzer.

---

## 📂 Repository Structure

/sender -> ESP32 sender (helmet) code
/receiver -> ESP32 receiver (base station) code
/hardware -> Circuit diagrams, Fritzing/Proteus files
/docs -> Project documentation, images



## 🖥️ How to Run

### 1. Clone this repo
```bash
git clone https://github.com/yourusername/intelli-helmet.git```

2. Install Libraries in Arduino IDE

i. WiFi.h (built-in with ESP32 board package)

ii. Wire.h (built-in)

iii. TinyGPSPlus

iv. Adafruit SSD1306 + Adafruit GFX

v. MPU6050 or Adafruit_MPU6050

vi. MAX3010x (SparkFun MAX3010x library)

vii. (Optional) TinyGsmClient for SIM800L

3. Upload Sender Code

i. Connect ESP32 (helmet) via USB.

ii. Select correct COM port.

iii. Upload /sender/sender.ino.

4. Upload Receiver Code

i. Connect second ESP32 (base) via USB.

ii. Select correct COM port.

iii. Upload /receiver/receiver.ino.

5. Wiring

i. Follow hardware/intelli-helmet_schematic.png for pin connections.

ii. Power SIM800L with a separate 4V stable supply capable of ≥2A bursts.

6. Test

i. Open Serial Monitor on both boards (115200 baud).

ii. Press panic switch or simulate a fall → watch OLED + buzzer on receiver.

⚙️ Configuration Tips

i. Edit ssid, password, receiverIP, and receiverPort in the sender code to match your network and receiver ESP32 IP.

ii. Adjust fall detection threshold constants in the code for your use case.

iii. Change pin numbers if using different ESP32 dev board layouts.

🤝 Contributing

Pull requests and feature suggestions are welcome!
If you build your own hardware variant, add wiring diagrams or PCB layouts in /hardware.

📄 License

This project is released under the MIT License.
You’re free to use, modify, and distribute with attribution.

📸 Demo



