// I2C_KEYPAD: para usar un GPIO expander PCF8574
// se usa la libreria:
// https://github.com/joeyoung/arduino_keypads/tree/master/Keypad_I2C
// si se comenta el #define, se usa el mapeo directo del keypad a los pines del Wemos
//#define I2C_KEYPAD

#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoOTA.h>
//LiquidCrystal_I2C lcd(0x27, 16, 2);
LiquidCrystal_I2C lcd(0x27, 20, 4);

#define SDA_PIN D2
#define SCL_PIN D1

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
char key;

#ifdef I2C_KEYPAD
#include "Keypad_I2C.h"
#define I2CADDR 0x20
byte rowPins[ROWS] = {0, 1, 2, 3}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 5, 6}; //connect to the column pinouts of the keypad
Keypad_I2C keypad = Keypad_I2C( makeKeymap(keys), rowPins, colPins, ROWS, COLS, I2CADDR );
#else
byte rowPins[ROWS] = {D7, D6, D5, D0} ;
byte colPins[COLS] = { D8, D4, D3} ;
#include <Keypad.h>
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
#endif

#include <Ticker.h>
Ticker tickerKey;
//#define AP
#ifndef AP
#define STASSID "MYSSID"
#define STAPSK  "MYPASS"
const char* host = "192.168.1.14";
#else
#define STASSID "ESP32go"
//#define STASSID "ESP-PGT"
#define STAPSK  "boquerones"
const char* host = "192.168.4.1";
#endif

WiFiClient client;
const char* ssid     = STASSID;
const char* password = STAPSK;
bool bklight = true;
bool focusF = false;
const uint16_t port = 10001;
void InitOTA()
{ ArduinoOTA.setHostname("wifipad.local");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
  });
  ArduinoOTA.onEnd([]() { });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) { });
  ArduinoOTA.onError([](ota_error_t error) { });
  ArduinoOTA.begin();

}

void keypadEvent(KeypadEvent key) {
  switch (keypad.getState()) {
    case PRESSED:
      if (client.connected()) {
        lcd.setCursor(11, 0);
        switch  (key) {
          case '4': client.print(":Mw#"); lcd.print("West  "); break;
          case '6': client.print(":Me#"); lcd.print("East  ") ; break;
          case '1': client.print(":F+#"); lcd.print("Focus+") ; break;
          case '3': client.print(":F-#"); lcd.print("Focus-") ; break;
          case '2': client.print(":Mn#"); lcd.print("North "); break;
          case '8': client.print(":Ms#"); lcd.print("South "); break;
          case '#': client.print(":RS#"); lcd.setCursor(11, 1); lcd.print("Slew  "); break;
          case '*': client.print(":RC#"); lcd.setCursor(11, 1); lcd.print("Center"); break;
          case '0':  client.print(":RG#");  lcd.setCursor(11, 1); lcd.print("Guide "); break;
          case '9':  client.print(":RM#");  lcd.setCursor(11, 1); lcd.print("Find "); break;
          case '7':  lcd.setCursor(0, 2); if (focusF) {
              client.print(":FF#");
               lcd.print("FastF ");
            } else {
              client.print(":FS#");
              lcd.print("SlowF ");
            }; focusF = !focusF; break;
          case '5': if (bklight) lcd.backlight() ; else lcd.noBacklight(); bklight = !bklight ; break;
          default: break;
        }


      }
      break;

    case RELEASED:
      if (client.connected()) {
        switch  (key) {
          case '4': client.print(":Qw#"); break;
          case '6': client.print(":Qe#"); break;
          case '2': client.print(":Qn#"); break;
          case '8': client.print(":Qs#"); break;
          case '3': client.print(":FQ#"); break;
          case '5': client.print(":FQ#"); break;
          default: break;

        }
        lcd.setCursor(11, 0); lcd.print("Track ");
      }
      break;

    case HOLD:
      if (key == '9') {
        Serial.println("hold");
      }
      break;
  }
}

void readKey() {
  key = keypad.getKey();
  if (key) {
    Serial.println(key);
  }
}


void setup() {
#ifdef I2C_KEYPAD
  Wire.begin(SDA_PIN, SCL_PIN);
#endif
  lcd.init();                      // initialize the lcd
  lcd.init();
  Serial.begin(115200);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("ESP32go Wctrl");
  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
#ifndef AP
  IPAddress ip(192, 168, 1, 35);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 0, 0);
  IPAddress DNS(192, 168, 1, 1);
#else
  IPAddress ip(192, 168, 4, 8);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 0, 0);
  IPAddress DNS(192, 168, 4, 1);
#endif
  WiFi.config(ip, gateway, subnet, gateway);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print("Wifi status: ");
    Serial.println(WiFi.status());
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("ESP32go WifiPad");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(3000);
 lcd.clear();
  // lcd.setCursor(0, 0);
  // lcd.print("RA:");
  //  lcd.setCursor(0, 1);
  //  lcd.print("DE:");

#ifdef I2C_KEYPAD
  keypad.begin();
#endif
  keypad.addEventListener(keypadEvent); // Add an event listener for this keypad
  tickerKey.attach_ms(100, readKey);


  InitOTA();
}


void loop() {
  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);
  String s;
  // Use WiFiClient class to create TCP connections
  lcd.setCursor(0, 0);
  lcd.print("Connecting....");
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    delay(5000);
    return;
  }
  lcd.setCursor(0,0);
  lcd.print("Connected.");
  delay(1000);
  if (client.connected())client.print(":IP"+String(WiFi.localIP()[3])+"."+String(WiFi.localIP()[2])+"#");
  lcd.clear();
  while (1) {
    // This will send a string to the server
    // Serial.println("send");
    unsigned long timeouts = millis();
    if (client.connected()) {
      client.flush();
      client.print(":GR#:GD#");
      lcd.setCursor(19, 0);
      lcd.print("S");
      if (Serial.available()) {
        s = Serial.readString();
        client.print(s);
        //  lcd.setCursor(1,1);
        // lcd.print(s);
      }
      delay(100);
      Serial.println(millis() - timeouts);
      // wait for data to be available
      unsigned long timeout = millis();
      while (client.available() == 0) {
        if (millis() - timeout > 5000) {
          Serial.println(">>> Client Timeout !");
          client.stop();
          delay(6000);
          return;
        }
      }


      char count = 0;
      s = "";
      while (client.available()) {
        char ch = static_cast<char>(client.read());
        s = s + ch;
      } client.flush();
      Serial.println(s);
      s.setCharAt(2, 'h');
      s.setCharAt(5, 'm');
      s.setCharAt(8, 's');
      s.setCharAt(14, 223);
      s.setCharAt(17, '\'');
      s.setCharAt(21, '"');
      lcd.setCursor(19, 0);
      lcd.print("X");
      s.replace("#", " ");
      s.replace(225, ':');
      // Serial.print(s);
      if (s.length() < 22) {
        lcd.setCursor(1, 0);
        lcd.print(s.substring(0, 9));
        lcd.setCursor(0, 1);
        lcd.print(s.substring(11, 20));
      } else Serial.print(s.length());

      //   Serial.println(millis() - timeouts);
    }

    // Close the connection
    //Serial.println("closing connection");
    //  client.stop();
    ArduinoOTA.handle();
    delay(500); //
  }
}



/*   keypad
   col   1 NC  2 GPIO13  3 GPIO12 4 GPIO14 5 GPIO16
   row   6 GPIO0 7 GPIO02 8 GPIO15
   lcd

*/
