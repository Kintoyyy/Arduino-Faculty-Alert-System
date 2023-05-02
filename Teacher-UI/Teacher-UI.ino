#include <LiquidCrystal_I2C.h>
#include <esp_now.h>
#include <WiFi.h>


#define AVAILABLE 19
#define BUSY 23
#define NOAVAILABLE 4


#define BUZZER 5

#define RED 17
#define ORANGE 16
#define GREEN 26


int displayIndex = 0;
int reqtype = 0;
String section;


int modeIndex = 0;  // 0 = available, 1 = busy, 2 = not available
const int modeSize = 4;
const char *modeItems[] = { "None", "Available", "Busy", "Not Around" };

int timeIndex = 0;  // 0 = available, 1 = busy, 2 = not available
const int timeSize = 4;
const char *timeItems[] = { "20 minutes", "40 minutes", "60 minutes", "80 minutes" };


LiquidCrystal_I2C lcd(0x27, 20, 4);

unsigned long previousMillis = 0;  // the last time the delay was updated
unsigned long interval = 5000;     // the interval (in milliseconds) between delays
unsigned long currentMillis;

// SENDING DATA ----------------------------------------------------------
// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = { 0x3C, 0x61, 0x05, 0x1F, 0x6B, 0x78 };

typedef struct struct_message {
  char section[50];
  int type;
  int response;
} struct_message;

struct_message myData;

esp_now_peer_info_t peerInfo;


void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  // mode buttons
  pinMode(AVAILABLE, INPUT_PULLUP);
  pinMode(BUSY, INPUT_PULLUP);
  pinMode(NOAVAILABLE, INPUT_PULLUP);
  //mode led
  pinMode(RED, OUTPUT);
  pinMode(ORANGE, OUTPUT);
  pinMode(GREEN, OUTPUT);

  pinMode(BUZZER, OUTPUT);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  lcd.init();
  lcd.backlight();

  // WiFi.mode(WIFI_MODE_STA);
  Serial.print("[?] Device Mac Address: ");
  Serial.println(WiFi.macAddress());

  lcd.setCursor(1, 1);
  lcd.print("Device Mac Address");
  lcd.setCursor(1, 2);
  lcd.print(WiFi.macAddress());

  delay(3000);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("[!] Error initializing ESP-NOW");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("[!] Error init ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("[!] Failed to add peer");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Failed to add peer");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  lcd.clear();
  lcd.setCursor(6, 1);
  lcd.print("Device Ready");
  delay(1000);
  lcd.clear();
}

void loop() {
  display();
  proccessInputs();

  if (modeIndex == 1) {
    digitalWrite(GREEN, HIGH);
    digitalWrite(ORANGE, LOW);
    digitalWrite(RED, LOW);

  } else if (modeIndex == 2) {
    digitalWrite(GREEN, LOW);
    digitalWrite(ORANGE, HIGH);
    digitalWrite(RED, LOW);

  } else if (modeIndex == 3) {
    digitalWrite(GREEN, LOW);
    digitalWrite(ORANGE, LOW);
    digitalWrite(RED, HIGH);

  } else {
    digitalWrite(GREEN, LOW);
    digitalWrite(ORANGE, LOW);
    digitalWrite(RED, LOW);
  }
}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "[?] Delivery Success" : "[!] Delivery Fail");
}

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {

  memcpy(&myData, incomingData, sizeof(myData));

  lcd.clear();
  displayIndex = 1;

  if (modeIndex == 0) {
    lcd.clear();
    display();
  } else {
    sendDataREsponse(modeIndex);
  }

  section = myData.section;
  reqtype = myData.type;

  display();

  Serial.println("[?] Current mode : " + (String)modeItems[modeIndex]);

  Serial.println("[->] Section : " + (String)section);


  switch (reqtype) {
    case 0:
      Serial.println("[-->] type : " + (String)reqtype + " = STUDENT");

      tone(BUZZER, 1000);
      delay(1000);
      noTone(BUZZER);
      break;
    case 1:
      Serial.println("[-->] type : " + (String)reqtype + " = PARENT");
      tone(BUZZER, 700);
      delay(1000);
      noTone(BUZZER);
      break;
    case 2:
      Serial.println("[-->] type : " + (String)reqtype + " = VISITOR");
      tone(BUZZER, 500);
      delay(1000);
      noTone(BUZZER);
      break;
    default:
      break;
  }
  if (modeIndex != 0) {
    delay(5000);
    lcd.clear();
    displayIndex = 0;
    display();
  }
}


void sendDataREsponse(int modee) {
  myData.response = modee;
  // Serial.println("Response mode: " + (String)modeIndex + " = " + (String)modeItems[modeIndex]);
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
}


void display() {

  if (displayIndex == 0) {
    // lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("CURRENT MODE:");
    lcd.setCursor((20 - (((String)modeItems[modeIndex]).length())) / 2, 1);
    lcd.print(modeItems[modeIndex]);

    if (modeIndex != 0) {
      lcd.setCursor(0, 3);
      lcd.print("click again to exit");
    } else {
      lcd.setCursor(4, 3);
      lcd.print("choose mode");
    }
  } else if (displayIndex == 1) {
    switch (reqtype) {
      case 0:
        lcd.setCursor(3, 0);
        lcd.print("STUDENT WAITING");
        lcd.setCursor(3, 2);
        lcd.print("From: " + (String)section);
        break;
      case 1:
        lcd.setCursor(2, 0);
        lcd.print("PARENT WAITING");
        lcd.setCursor(3, 2);
        lcd.print("From: " + (String)section);
        break;
      case 2:
        lcd.setCursor(3, 3);
        lcd.print("VISITOR WAITING");
        break;
      default:
        break;
    }
    if (modeIndex == 0) {
      lcd.setCursor(3, 3);
      lcd.print("Select Response");
    }
  }
}


void proccessInputs() {
  while (digitalRead(AVAILABLE) == HIGH && digitalRead(BUSY) == HIGH && digitalRead(NOAVAILABLE) == HIGH)
    ;
  lcd.clear();

  if (digitalRead(AVAILABLE) == LOW) {

    if (displayIndex == 0) {

      if (modeIndex != 1) {
        modeIndex = 1;
      } else {
        modeIndex = 0;
      }

    } else if (displayIndex == 1) {
      if (modeIndex == 0) {
        modeIndex = 1;
        displayIndex = 0;
        sendDataREsponse(1);
      }
    }
  } else if (digitalRead(BUSY) == LOW) {

    if (displayIndex == 0) {

      if (modeIndex != 2) {
        modeIndex = 2;
      } else {
        modeIndex = 0;
      }

    } else if (displayIndex == 1) {
      if (modeIndex == 0) {
        modeIndex = 2;
        displayIndex = 0;
        sendDataREsponse(2);
      }
    }
  } else if (digitalRead(NOAVAILABLE) == LOW) {

    if (displayIndex == 0) {
      if (modeIndex != 3) {
        modeIndex = 3;
      } else {
        modeIndex = 0;
      }

    } else if (displayIndex == 1) {
      if (modeIndex == 0) {
        modeIndex = 3;
        displayIndex = 0;
        sendDataREsponse(3);
      }
    }
  }

  // Serial.println("displayIndex: " + (String)displayIndex);
  // Serial.println("modeIndex: " + (String)modeIndex);

  // Serial.println("Selected: " + (String)modeItems[modeIndex]);
  while (digitalRead(AVAILABLE) == LOW || digitalRead(BUSY) == LOW || digitalRead(NOAVAILABLE) == LOW)
    ;
}
