
#define BLYNK_TEMPLATE_ID "TMPL6_Id1Clvh"
#define BLYNK_TEMPLATE_NAME "Project Smart Home"
#define BLYNK_AUTH_TOKEN "1ffkgzY-d-ACF_2pLSnZ3hPdKF4aS2GJ"

#include <ESP8266WiFi.h>   
#include <BlynkSimpleEsp8266.h>  
#include <DHT.h>        
#include <Servo.h>       

char ssid[] = "Galaxy A14 2817";
char pass[] = "dracomalfoyteamo";

#define DHTPIN D2         
#define DHTTYPE DHT11     
#define PIR_PIN D1       
#define LDR_PIN A0        
#define LED_TAMAN D5    
#define LED_RUMAH D6    
#define SERVO_PIN D7   

DHT dht(DHTPIN, DHTTYPE);
Servo myServo;
BlynkTimer timer;

unsigned long waktuTerakhirGerakan = 0;  
const unsigned long durasiDelay = 5000;   

bool lampuRumahNyala = false;
bool lampuTamanNyala = false;
bool servoTerbuka = false;


bool kontrolManualRumah = false;
bool kontrolManualTaman = false;
bool statusManualLampuRumah = false;
bool statusManualLampuTaman = false;


float suhu = 0;
int nilaiLDR = 0;
int pirVal;

const float ambangSuhu = 25.0;  
const int ambangLDR = 300;     

void setup() {
  Serial.begin(115200);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  pinMode(PIR_PIN, INPUT);
  pinMode(LED_TAMAN, OUTPUT);
  pinMode(LED_RUMAH, OUTPUT);

  dht.begin();
  myServo.attach(SERVO_PIN);
  myServo.write(0);

 
  timer.setInterval(2000L, bacaSensorDHT);   
  timer.setInterval(200L, bacaSensorPIR);     
  timer.setInterval(2000L, bacaSensorLDR);    
  timer.setInterval(2000L, kontrolServo);      
  timer.setInterval(3000L, kirimKeBlynk);     
  timer.setInterval(3000L, logKeSerialMonitor);

  Serial.println("=== SISTEM RUMAH PINTAR TERHUBUNG KE BLYNK ===");
}

void loop() {
  Blynk.run(); 
  timer.run(); 
}

void bacaSensorDHT() {
  float suhuBaca = dht.readTemperature();
  if (!isnan(suhuBaca)) {
    suhu = suhuBaca;
  }
}

void bacaSensorPIR() {
  if (kontrolManualRumah) {
    digitalWrite(LED_RUMAH, statusManualLampuRumah);
    return; 
  }

  pirVal = digitalRead(PIR_PIN);
  if (pirVal == HIGH) { 
    digitalWrite(LED_RUMAH, HIGH);
    if (!lampuRumahNyala) {
      lampuRumahNyala = true;
      Blynk.virtualWrite(V1, "Gerakan Terdeteksi");
      Blynk.virtualWrite(V4, 255); 
    }
    waktuTerakhirGerakan = millis(); 
  } else { 
    if (millis() - waktuTerakhirGerakan > durasiDelay && lampuRumahNyala) {
      digitalWrite(LED_RUMAH, LOW);
      lampuRumahNyala = false;
      Blynk.virtualWrite(V1, "Tidak Ada Gerakan");
      Blynk.virtualWrite(V4, 0); 
    }
  }
}

void bacaSensorLDR() {
  nilaiLDR = analogRead(LDR_PIN);
  Blynk.virtualWrite(V2, nilaiLDR); 

  if (kontrolManualTaman) {
    digitalWrite(LED_TAMAN, statusManualLampuTaman);
    return; 
  }

  if (nilaiLDR >= ambangLDR && !lampuTamanNyala) { 
    digitalWrite(LED_TAMAN, HIGH);
    lampuTamanNyala = true;
    Blynk.virtualWrite(V5, 255); 
  } else if (nilaiLDR < ambangLDR && lampuTamanNyala) { 
    digitalWrite(LED_TAMAN, LOW);
    lampuTamanNyala = false;
    Blynk.virtualWrite(V5, 0);
  }
}

void kontrolServo() {
  bool siang = nilaiLDR < ambangLDR; 

  if (siang) {
    if (suhu > ambangSuhu && !servoTerbuka) {
      myServo.write(120); 
      servoTerbuka = true;
      Blynk.virtualWrite(V3, "TERBUKA (Siang, Panas)");
    } else if (suhu <= ambangSuhu && servoTerbuka) {
      myServo.write(0); 
      servoTerbuka = false;
      Blynk.virtualWrite(V3, "TERTUTUP (Siang, Sejuk)");
    }
  } else {
    
    if (servoTerbuka) {
      myServo.write(0); 
      servoTerbuka = false;
      Blynk.virtualWrite(V3, "TERTUTUP (Malam)");
    }
  }
}

void kirimKeBlynk() {
  Blynk.virtualWrite(V0, suhu); 
}

void logKeSerialMonitor() {
  Serial.println("======= STATUS SISTEM =======");
  Serial.print("Suhu: ");
  Serial.print(suhu);
  Serial.println(" °C");

  Serial.print("Status PIR: ");
  Serial.println(lampuRumahNyala ? "GERAKAN TERDETEKSI" : "TIDAK ADA GERAKAN");

  Serial.print("Nilai LDR: ");
  Serial.println(nilaiLDR);
  Serial.print("Lampu Taman: ");
  Serial.println(lampuTamanNyala ? "HIDUP (gelap)" : "MATI (terang)");

  Serial.print("Jendela: ");
  Serial.println(servoTerbuka ? "TERBUKA" : "TERTUTUP");

  Serial.println("=============================");
}

BLYNK_WRITE(V6) {
  int value = param.asInt(); 
  kontrolManualRumah = value;
  statusManualLampuRumah = value;
  digitalWrite(LED_RUMAH, value);
  Blynk.virtualWrite(V4, value ? 255 : 0);
}

BLYNK_WRITE(V7) {
  int value = param.asInt(); 
  kontrolManualTaman = value;
  statusManualLampuTaman = value;
  digitalWrite(LED_TAMAN, value);
  Blynk.virtualWrite(V5, value ? 255 : 0); 
}
