
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {D7, D6, D5, D0} ;
byte colPins[COLS] = { D8, D4, D3} ;
char key;
#include <Keypad.h>
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

#include <Ticker.h>
Ticker tickerKey;
#define AP
#ifndef AP
#define STASSID "wifi"
#define STAPSK  "passed"
const char* host = "192.168.1.82";
#else
#define STASSID "ESP32go"
#define STAPSK  "boquerones"
const char* host = "192.168.4.1";
#endif
WiFiClient client;
const char* ssid     = STASSID;
const char* password = STAPSK;


const uint16_t port = 10001;

void keypadEvent(KeypadEvent key) {
  switch (keypad.getState()) {
    case PRESSED:
      if (client.connected()) {
        switch  (key) {
          case '4': client.print(":Mw#"); break;
          case '6': client.print(":Me#"); break;
          case '2': client.print(":Mn#"); break;
          case '8': client.print(":Ms#"); break;
          case '#': client.print(":RS#"); break;
          case '*': client.print(":RC#"); break;
          case '0':  client.print(":RG#"); break;
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
          default: break;
        }
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
   IPAddress ip(192, 168, 4, 8);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 0, 0);
  //  IPAddress DNS(192, 168, 1, 1);
  WiFi.config(ip, gateway, subnet, gateway);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.println(WiFi.status());
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("ESP32go Wctrl");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RA:");
  lcd.setCursor(0, 1);
  lcd.print("DE:");
  keypad.addEventListener(keypadEvent); // Add an event listener for this keypad
  tickerKey.attach_ms(100, readKey);
}


void loop() {
  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);
  String s;
  // Use WiFiClient class to create TCP connections
  //  lcd.setCursor(1,1);
  //     lcd.print("test");
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    delay(5000);
    return;
  }
  while (1) {
    // This will send a string to the server
    Serial.println("send");
    unsigned long timeouts = millis();
    if (client.connected()) {
      client.flush();
      client.print(":GR#:GD#");

      if (Serial.available()) {
        s = Serial.readString();
        client.print(s);
        //  lcd.setCursor(1,1);
        // lcd.print(s);
      }
      delay(300);
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
      }client.flush();

      s.replace("#", " ");
      s.replace(225, ':');
     Serial.print(s);
      if (s.length() < 22){
      lcd.setCursor(4, 0);
      lcd.print(s.substring(0, 10));
      lcd.setCursor(3, 1);
      lcd.print(s.substring(11, 20));
    }else Serial.print(s.length());

   //   Serial.println(millis() - timeouts);
    }

    // Close the connection
     //Serial.println("closing connection");
    //  client.stop();

    delay(500); // 
}
