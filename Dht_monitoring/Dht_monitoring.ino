// =======================================================================
// BAGIAN 1: Sertakan semua library yang dibutuhkan
// =======================================================================

// Library untuk sensor DHT22
#include <DHT22.h> 

// Library untuk WiFi di ESP32
#include <WiFi.h>

// Library untuk HTTP Client (mengirim data ke server)
#include <HTTPClient.h>

// Library untuk LCD I2C
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// --- Library Baru untuk Termokopel MAX6675 ---
#include <max6675.h> 
// --- Akhir Library Baru ---

// =======================================================================
// BAGIAN 2: Konfigurasi
// =======================================================================

// Konfigurasi WiFi Anda (WAJIB DIUBAH)
const char* ssid = "admin";
const char* password = "admin123";

// Konfigurasi pin sensor DHT22 (untuk kelembaban)
#define DHT_PIN 27 // Ubah nama pin biar lebih jelas ini untuk DHT

// --- Konfigurasi pin Termokopel MAX6675 ---
const int thermoCLK = 18;  // SCK (SPI Clock)
const int thermoCS = 5;    // CS (Chip Select)
const int thermoDO = 23;   // SO (Serial Out / Data Output)
// --- Akhir Konfigurasi Termokopel ---

// Konfigurasi pin Relay
const int RLampu = 19;
const int RKipas = 14; // HATI-HATI! Pin 18 sudah dipakai thermoCLK. Ubah RKipas ke pin lain yang kosong, misalnya 14.
const int RHumi = 2;   // HATI-HATI! Pin 5 sudah dipakai thermoCS. Ubah RHumi ke pin lain yang kosong, misalnya 2.

// Konfigurasi LCD I2C
int lcdAddress = 0x27; 
int lcdColumns = 16;
int lcdRows = 2;

const char* serverUrl = "http://10.187.20.157:8000/api/dht/store";

// =======================================================================
// BAGIAN 3: Inisialisasi Objek
// =======================================================================

// Buat objek sensor dht22 (Hanya untuk kelembaban)
DHT22 dht22(DHT_PIN); 

// --- Buat objek Termokopel MAX6675 ---
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
// --- Akhir Objek Termokopel ---

// Buat objek LCD
LiquidCrystal_I2C lcd(lcdAddress, lcdColumns, lcdRows);

// Variabel untuk timing
unsigned long lastSensorReadTime = 0;
const long sensorReadInterval = 5000; // Interval pembacaan sensor dan pengiriman data (5 detik)

unsigned long lastLcdUpdateTime = 0;
const long lcdUpdateInterval = 2000; // Interval pembaruan LCD (2 detik)

// Variabel global untuk menyimpan data sensor yang terakhir dibaca
float currentTemperature = 0.0; // Ini akan dari Termokopel
float currentHumidity = 0.0;    // Ini akan dari DHT22
bool isWiFiConnected = false;
bool isDhtReadOk = false;
bool isThermoReadOk = false; // Status baca termokopel

// =======================================================================
// FUNGSI PROTOTYPE
// =======================================================================
void sendData(float temp, float hum);
void updateLcdDisplay();
void controlRelays(float temp);

// =======================================================================
// FUNGSI SETUP
// =======================================================================
void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Smart Environment Monitor (Thermo+DHT)");

  // Inisialisasi pin Relay sebagai OUTPUT
  pinMode(RLampu, OUTPUT);
  pinMode(RKipas, OUTPUT);
  pinMode(RHumi, OUTPUT);

  // Pastikan semua relay mati di awal
  digitalWrite(RLampu, LOW);
  digitalWrite(RKipas, LOW);
  digitalWrite(RHumi, LOW);

  // Inisialisasi LCD
  Wire.begin(); 
  lcd.init(); 
  lcd.backlight(); 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to");
  lcd.setCursor(0, 1);
  lcd.print("WiFi...");
  
  // Hubungkan ke WiFi
  WiFi.begin(ssid, password);
  int connectAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && connectAttempts < 20) {
    delay(500);
    Serial.print(".");
    connectAttempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    isWiFiConnected = true;
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    delay(2000); 
  } else {
    isWiFiConnected = false;
    Serial.println("\nWiFi Failed!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed!");
    lcd.setCursor(0, 1);
    lcd.print("Check credentials");
    delay(5000); 
  }

  // Inisialisasi sensor DHT22
  // dht22.begin(); 
  Serial.println("DHT22 Sensor Initialized.");
  
  // Termokopel tidak memerlukan 'begin()', pembacaan pertama akan menginisialisasi
  Serial.println("MAX6675 Thermocouple Initialized.");

  lcd.clear(); 
}

// =======================================================================
// FUNGSI LOOP
// =======================================================================
void loop() {
  unsigned long currentMillis = millis();

  // --- Bagian Pembacaan Sensor dan Pengiriman Data ---
  if (currentMillis - lastSensorReadTime >= sensorReadInterval) {
    lastSensorReadTime = currentMillis;

    // Baca Suhu dari MAX6675
    float newTemperature = thermocouple.readCelsius(); 
    if (isnan(newTemperature) || newTemperature < -200 || newTemperature > 1300) { // Cek nilai valid, MAX6675 bisa beri 0 jika disconnected
      isThermoReadOk = false;
      Serial.println("Failed to read from MAX6675 (check wiring/connected).");
      // Optional: bisa pakai last good value atau N/A
    } else {
      currentTemperature = newTemperature;
      isThermoReadOk = true;
    }

    // Baca Kelembaban dari DHT22
    float newHumidity = dht22.getHumidity();
    if (dht22.getLastError() == DHT22::OK) {
      currentHumidity = newHumidity;
      isDhtReadOk = true;
    } else {
      isDhtReadOk = false;
      Serial.print("Failed to read humidity from DHT22, error: ");
      Serial.println(dht22.getLastError());
    }

    // Kirim data hanya jika kedua sensor berhasil membaca atau setidaknya satu sensor utama berhasil (tergantung prioritas)
    // Di sini kita kirim selama suhu dari termokopel berhasil terbaca, kelembaban mungkin N/A jika DHT error
    if (isThermoReadOk) { // Kirim data jika suhu berhasil dibaca
      Serial.printf("Read: Temp: %.1f *C (MAX6675), Hum: %.1f %% (DHT22)\n", currentTemperature, currentHumidity);
      if (isWiFiConnected) {
        sendData(currentTemperature, currentHumidity); 
      } else {
        Serial.println("WiFi not connected, skipping data send.");
      }
      
      // Kontrol relay berdasarkan suhu terbaru dari termokopel
      controlRelays(currentTemperature);

    } else {
      Serial.println("Skipping data send and relay control due to MAX6675 read error.");
    }
  }

  // --- Bagian Pembaruan LCD ---
  if (currentMillis - lastLcdUpdateTime >= lcdUpdateInterval) {
    lastLcdUpdateTime = currentMillis;
    updateLcdDisplay(); 
  }
}

// =======================================================================
// FUNGSI sendData: Mengirim data suhu dan kelembaban ke server Laravel
// =======================================================================
void sendData(float temp, float hum) {
  if (WiFi.isConnected()) { 
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    String httpRequestData = "{\"temperature\":" + String(temp, 2) + ",\"humidity\":" + String(hum, 2) + "}";
    Serial.print("Sending JSON: ");
    Serial.println(httpRequestData);

    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String response = http.getString();
      Serial.println("Server Response: ");
      Serial.println(response);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      Serial.println(http.errorToString(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("WiFi not connected. Cannot send data.");
    isWiFiConnected = (WiFi.status() == WL_CONNECTED); // Update status koneksi
  }
}

// =======================================================================
// FUNGSI updateLcdDisplay: Memperbarui tampilan pada LCD
// =======================================================================
void updateLcdDisplay() {
  lcd.clear(); 

  // Update status WiFi
  if (WiFi.status() != WL_CONNECTED) {
      isWiFiConnected = false;
      WiFi.reconnect(); // Coba reconnect
  } else {
      isWiFiConnected = true;
  }

  if (!isWiFiConnected) {
    lcd.setCursor(0, 0);
    lcd.print("WiFi Diconnected!");
    lcd.setCursor(0, 1);
    lcd.print("Reconnecting...");
    return; 
  }

  if (!isThermoReadOk && !isDhtReadOk) { // Jika kedua sensor bermasalah
    lcd.setCursor(0, 0);
    lcd.print("Sensor Errors!");
    lcd.setCursor(0, 1);
    lcd.print("Check Wiring!");
    return;
  }

  // Tampilkan Suhu dan Kelembaban (sesuaikan dengan status baca)
  lcd.setCursor(0, 0);
  lcd.print("S:");
  if (isThermoReadOk) {
    lcd.print(currentTemperature, 1); 
    lcd.print("C");
  } else {
    lcd.print("N/A C");
  }
  // lcd.print(" H:");
  if (isDhtReadOk) {
    // lcd.print(currentHumidity, 1);
    // lcd.print("%");
  } else {
    // lcd.print("N/A %");
  }


  // Tampilkan Status Relay di baris kedua
  lcd.setCursor(0, 1);
  lcd.print("L:");
  lcd.print(digitalRead(RLampu) == HIGH ? "ON" : "OFF");
  lcd.print(" K:");
  lcd.print(digitalRead(RKipas) == HIGH ? "ON" : "OFF");
  // lcd.print(" Hu:");
  // lcd.print(digitalRead(RHumi) == HIGH ? "ON" : "OFF");
}

// =======================================================================
// FUNGSI controlRelays: Mengontrol relay berdasarkan suhu
// =======================================================================
void controlRelays(float t) {
  // Logika RLampu
  if (t < 32.00) {
    digitalWrite(RLampu, HIGH);
    Serial.println("RLampu: ON (Temp < 32)");
  } else { // if (t >= 32.00)
    digitalWrite(RLampu, LOW);
    Serial.println("RLampu: OFF (Temp >= 32)");
  }

  // Logika RKipas dan RHumi
  if (t > 32.00) {
    digitalWrite(RKipas, HIGH);
    digitalWrite(RHumi, HIGH);
    Serial.println("RKipas: ON, RHumi: ON (Temp > 32)");
  } else { // if (t <= 32.00)
    digitalWrite(RKipas, LOW);
    digitalWrite(RHumi, LOW);
    Serial.println("RKipas: OFF, RHumi: OFF (Temp <= 32)");
  }
}