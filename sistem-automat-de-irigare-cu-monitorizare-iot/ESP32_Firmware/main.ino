#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


const char* ssid = "Top G";
const char* password = "minimopt";
const char* serverUrl = "http://192.168.0.124:5125/api/Senzor";


#define PIN_SENZOR_SOL 34 
#define PIN_DHT        4 
#define PIN_PLOAIE     5 
#define PIN_RELEU      26

//  CALIBRARE SENZOR SOL
// Pune senzorul in aer liber si noteaza valoarea bruta -> SOL_USCAT
// Pune senzorul in pamant ud si noteaza valoarea bruta -> SOL_UD
#define SOL_USCAT 3200   // valoare bruta in aer / pamant uscat
#define SOL_UD    1000   // valoare bruta in pamant ud

//  PRAGURI 
#define PRAG_PORNIRE  40  // % - sub acest prag porneste pompa
#define PRAG_OPRIRE   50  // % - peste acest prag opreste pompa

//  TIMPI 
#define DELAY_PORNIRE_POMPA  60000   // 1 minut in ms - cat trebuie sa fie uscat inainte sa porneasca
#define DELAY_PLOAIE        180000   // 3 minute in ms - cat trebuie sa ploua ca sa opreasca pompa

// ============ DHT ============
#define DHT_TIP DHT22
DHT dht(PIN_DHT, DHT_TIP);

// ============ OLED ============
#define LATIME_OLED  128
#define INALTIME_OLED 64
Adafruit_SSD1306 display(LATIME_OLED, INALTIME_OLED, &Wire, -1);

// ============ VARIABILE GLOBALE ============
bool pompaPornita        = false;
bool ploaieDetectata     = false;

unsigned long timpStartUscat  = 0;   // cand a inceput sa fie uscat
unsigned long timpStartPloaie = 0;   // cand a inceput sa ploua
bool soculUscatInceput        = false;
bool soculPloaieInceput       = false;

unsigned long timpUltimaPagina = 0;
int paginaCurenta              = 1;

float temperatura    = 0;
float umiditateAer   = 0;
int umiditateSol     = 0;
int valBrutaSol      = 0;
unsigned long ultimulPost = 0; // Timer pentru trimitere date

//  MODUL AUTO/MANUAL 
bool modAuto = true; // Implicit, ESP-ul este pe modul automat
// ----------------------------------------


//  SETUP 
void setup() {
  Serial.begin(115200);

  pinMode(PIN_RELEU, OUTPUT);
  digitalWrite(PIN_RELEU, HIGH); // HIGH = releu oprit

  pinMode(PIN_PLOAIE, INPUT);

  dht.begin();
  Wire.begin();
    Wire.setTimeOut(100);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED negasit!");
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("Conectare WiFi...");
    display.display();
  }

  // Conectare WiFi
  WiFi.begin(ssid, password);
  Serial.print("Se conecteaza la WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Conectat!");

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Sistem pornit!");
  display.println("WiFi OK");
  display.display();
  delay(2000);
}

//  LOOP 
void loop() {
  unsigned long acum = millis();

  //  Citire senzori 
  valBrutaSol    = analogRead(PIN_SENZOR_SOL);
  umiditateSol   = map(valBrutaSol, SOL_USCAT, SOL_UD, 0, 100);
  umiditateSol   = constrain(umiditateSol, 0, 100);

  temperatura  = dht.readTemperature();
  umiditateAer = dht.readHumidity();

  bool ploaieAcum = (digitalRead(PIN_PLOAIE) == LOW);

  //  Logica ploaie cu delay 3 minute 
  if (ploaieAcum) {
    if (!soculPloaieInceput) {
      soculPloaieInceput = true;
      timpStartPloaie    = acum;
    }
    if (acum - timpStartPloaie >= DELAY_PLOAIE) {
      ploaieDetectata = true;
    }
  } else {
    soculPloaieInceput = false;
    ploaieDetectata    = false;
  }

  //  Logica pornire/oprire pompa (ADAPTATĂ PENTRU AUTO/MANUAL) 
  if (!modAuto) {
    // MOD MANUAL: ESP-ul ignora senzorii. Pompa asculta exclusiv de variabila venita de pe Web
    digitalWrite(PIN_RELEU, pompaPornita ? LOW : HIGH);
    soculUscatInceput = false; // Resetam timerul automat ca sa nu ne incurce la revenirea in Auto
  } else {
    // MOD AUTOMAT
    if (ploaieDetectata) {
      // Ploua — opreste pompa si reseteaza timerul
      pompaPornita       = false;
      soculUscatInceput  = false;
      digitalWrite(PIN_RELEU, HIGH);
       Serial.println("Ploua - pompa oprita"); 

    } else if (!pompaPornita) {
      // Pompa e oprita — verifica daca e uscat destul de mult
      if (umiditateSol < PRAG_PORNIRE) {
        if (!soculUscatInceput) {
          soculUscatInceput = true;
          timpStartUscat    = acum;
        }
        // A trecut 1 minut de cand e uscat?
        if (acum - timpStartUscat >= DELAY_PORNIRE_POMPA) {
          pompaPornita = true;
          digitalWrite(PIN_RELEU, LOW);
        }
      } else {
        // Sol umed — reseteaza timerul
        soculUscatInceput = false;
      }
    } else {
      // Pompa e pornita — opreste doar daca depaseste pragul de oprire
      if (umiditateSol > PRAG_OPRIRE) {
        pompaPornita = false;
        digitalWrite(PIN_RELEU, HIGH);
        soculUscatInceput = false;
      }
    }
  }
  

  //  Trimitere date catre Server la fiecare 5 secunde 
  if (acum - ultimulPost >= 5000) {
    ultimulPost = acum;
    
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverUrl);
      http.addHeader("Content-Type", "application/json");

      // Creare pachet JSON
      StaticJsonDocument<200> doc;
      doc["UmiditateSol"] = umiditateSol;
      doc["Temperatura"] = isnan(temperatura) ? 0 : temperatura;
      doc["UmiditateAer"] = isnan(umiditateAer) ? 0 : umiditateAer;
      doc["Ploaie"] = ploaieDetectata;
      doc["PompaActiva"] = pompaPornita;

      String requestBody;
      serializeJson(doc, requestBody);

      // Trimitere POST si citire raspuns HTTP
      int httpResponseCode = http.POST(requestBody);
      
      if (httpResponseCode > 0) {
        String payload = http.getString();
        
        // Decodam raspunsul JSON primit de la C#
        StaticJsonDocument<200> docResp;
        deserializeJson(docResp, payload);
        
        // Preluam modul de operare de pe web (Auto/Manual)
        if (docResp.containsKey("modAuto")) {
          modAuto = docResp["modAuto"];
        }
        
        // Preluam comanda pompei de pe web doar daca s-a apasat butonul
        if (docResp["areComandaNoua"] == true) {
          pompaPornita = docResp["comandaPompa"];
          Serial.print("COMANDA INTERNET PRIMITA! Mod Auto: ");
          Serial.println(modAuto ? "DA" : "NU");
        }
      } else {
        Serial.print("Eroare trimitere HTTP: ");
        Serial.println(httpResponseCode);
      }
      http.end();
    }
  }

  //  Schimbare pagina OLED la 4 secunde 
  if (acum - timpUltimaPagina >= 4000) {
    timpUltimaPagina = acum;
    paginaCurenta    = (paginaCurenta == 1) ? 2 : 1;
  }

  //  Afisare OLED 
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  if (paginaCurenta == 1) {
    // Pagina 1 — Sol + Pompa + Timer
    display.setCursor(0, 0);
    display.print("-- IRIGARE -- "); 
    display.println(modAuto ? "[AUTO]" : "[MAN]"); // Aratam modul pe ecran

    display.setCursor(0, 16);
    display.print("Sol: "); display.print(umiditateSol); display.println("%");

    display.setCursor(0, 32);
    display.print("Pompa: "); display.println(pompaPornita ? "PORNITA" : "OPRITA");

    display.setCursor(0, 48);
    if (!pompaPornita && soculUscatInceput && !ploaieDetectata && modAuto) {
      unsigned long ramas = (DELAY_PORNIRE_POMPA - (acum - timpStartUscat)) / 1000;
      display.print("Pornire in: "); display.print(ramas); display.print("s");
    } else if (ploaieDetectata && modAuto) {
      display.println("Ploua!");
    } else if (!modAuto) {
      display.println("Control Manual Web");
    } else {
      display.println("Sol OK");
    }

  } else {
    // Pagina 2 — Temperatura + Umiditate aer + Ploaie
    display.setCursor(0, 0);
    display.println("-- CLIMAT --");

    display.setCursor(0, 16);
    display.print("Temp: "); display.print(temperatura, 1); display.println(" C");

    display.setCursor(0, 32);
    display.print("Aer: "); display.print(umiditateAer, 1); display.println("%");

    display.setCursor(0, 48);
    display.print("Ploaie: "); display.println(ploaieDetectata ? "DA - activa" : "NU");
  }

  display.display();

  delay(100); // Pentru stabilitatea buclei cu WiFi activ
}