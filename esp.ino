#include <WiFi.h>
#include <HTTPClient.h>

#define TRIG_PIN 18
#define ECHO_PIN 19
#define RELAY_PIN 27
#define FLOW_SENSOR_PIN 17 // Pin untuk sensor water flow

long duration;
float distance;
float waterLevel;

const float tankHeight = 19.0;

// Variabel untuk water flow
volatile int pulseCount = 0; 
float flowRate = 0;
float totalLiters = 0;

// Koefisien sesuai spesifikasi sensor
const float calibrationFactor = 450.0; // Ubah sesuai spesifikasi sensor Anda

// WiFi credentials
const char* ssid = "yourWIFI";
const char* password = "WifiPassword";
const char* host = "http://yourIPAddress/project/api.php";

// State pompa
bool pumpState = false; // false = OFF, true = ON

// Interrupt untuk sensor water flow
void IRAM_ATTR pulseCounter() {
    pulseCount++;
}

void setup() {
    Serial.begin(115200);
    
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);

    digitalWrite(RELAY_PIN, HIGH); // Atur relay ke OFF jika aktif rendah
    
    // Attach interrupt untuk sensor water flow
    attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, FALLING);

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
}

void loop() {
    // Mengukur jarak dengan sensor ultrasonik
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
  
    duration = pulseIn(ECHO_PIN, HIGH, 30000); // Tambahkan timeout
    if (duration == 0) {
        Serial.println("Sensor timeout");
        return; // Lewati iterasi jika sensor timeout
    }

    distance = (duration * 0.0343) / 2;
    waterLevel = tankHeight - distance;

    Serial.print("Water Level: ");
    Serial.print(waterLevel);
    Serial.println(" cm");

    // Logika ambang batas bawah dan atas
    if (waterLevel <= 5 && !pumpState) { 
        pumpState = true; 
        digitalWrite(RELAY_PIN, LOW); 
        Serial.println("Pump ON");
    } else if (waterLevel >= 15 && pumpState) {
        pumpState = false; 
        digitalWrite(RELAY_PIN, HIGH); 
        Serial.println("Pump OFF");
    }

    // Hitung flow rate dan total volume
    detachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN));
    flowRate = (pulseCount / calibrationFactor) / (500 / 1000.0); 
    totalLiters += (flowRate / 60.0) * 1.0;
    pulseCount = 0; 
    attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, FALLING);

    Serial.print("Flow Rate: ");
    Serial.print(flowRate);
    Serial.println(" L/min");

    Serial.print("Total Volume: ");
    Serial.print(totalLiters);
    Serial.println(" L");

    // Kirim data ke server
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(host);

        String postData = String("{\"waterLevel\":") + waterLevel +
                          ",\"flowRate\":" + flowRate +
                          ",\"totalLiters\":" + totalLiters + "}";

        http.addHeader("Content-Type", "application/json");
        int httpResponseCode = http.POST(postData);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Server Response: " + response);
        } else {
            Serial.println("Error sending data");
        }
        http.end();
    }

    delay(1000);
}
