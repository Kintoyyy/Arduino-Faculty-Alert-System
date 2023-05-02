#include <LiquidCrystal_I2C.h>
#include <esp_now.h>
#include <WiFi.h>


#define UP 19
#define DOWN 23
#define ENTER 18
#define BACK 5

#define RED 16
#define ORANGE 17
#define GREEN 26


LiquidCrystal_I2C lcd(0x27, 20, 4);

// DATA PROCCESING ---------------------------------------------------------------

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t nodeAddress1[] = { 0x08, 0x3A, 0xF2, 0x54, 0x19, 0xA0 };

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  char section[50];
  int type;
  int response;
} struct_message;

// Create a struct_message called Data
struct_message Data;

esp_now_peer_info_t peerInfo;


// NAVIGATION -------------------------------------------------------------------
int menu = 0;
int SelectedMenu = 0;
int selectedSection = 0;
int selectedTeacher = 0;


int menuIndex = 0;
const int mainMenuSize = 3;
const char* mainMenuItems[] = { "Student", "Parent", "Visitor" };

int sectionMenuIndex = 0;
const int sectionMenuSize = 2;
const char* sectionMenuItems[] = { "ST12P1", "ST12P3" };

int teacherMenuIndex = 0;
const int teacherMenuSize = 9;
const char* teacherMenuItems[] = { "RODRIGO", "JONI", "LOCHINVAR", "ETHEL JEANE", "XERXES", "CHARISSE", "NINA CARISSA", "KATHLEEN", "ALFIE" };


// Variables

int response = 0;

void setup() {
  Serial.begin(115200);

  // input buttons
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(ENTER, INPUT_PULLUP);
  pinMode(BACK, INPUT_PULLUP);
  // output leds
  pinMode(RED, OUTPUT);
  pinMode(ORANGE, OUTPUT);
  pinMode(GREEN, OUTPUT);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  lcd.init();
  lcd.backlight();

  Serial.print("[?] Device Mac Address: ");
  Serial.println(WiFi.macAddress());

  lcd.clear();
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
  memcpy(peerInfo.peer_addr, nodeAddress1, 6);
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

  esp_now_register_recv_cb(OnDataRecv);

  lcd.clear();
  lcd.setCursor(6, 1);
  lcd.print("Device Ready");
  delay(1000);
  lcd.clear();
}

void loop() {
  display();
  navigation();
}

void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
  // Serial.println("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "[?] Delivery Success" : "[!] Delivery Fail");
  if (status != ESP_NOW_SEND_SUCCESS) {
    digitalWrite(RED, HIGH);
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("CONNECTION ERROR");
    lcd.setCursor(2, 2);
    lcd.print("Device not Found");
    delay(5000);
    digitalWrite(RED, LOW);
  }
}


void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len) {

  response = 0;  // set back to 0 very response

  memcpy(&Data, incomingData, sizeof(Data));

  response = Data.response;

  Serial.println("[->] Data recieved : " + (String)response);

  lcd.clear();

  if (response == 0) {
    // open waiting response tab
    menu = 5;
  } else {
    menu = 4;
  }
  delay(100);
  display();
}

bool sendData() {
  switch (sectionMenuIndex) {
    case 0:
      strcpy(Data.section, "ST12P1");
      break;
    case 1:
      strcpy(Data.section, "ST12P3");
      break;
    default:
      break;
  }

  Data.type = menuIndex;

  Serial.println("[<] Type: " + (String)menuIndex);
  Serial.println("[<] Section: " + (String)Data.section);

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(nodeAddress1, (uint8_t*)&Data, sizeof(Data));
  if (result != ESP_OK) {
    Serial.println("[!] Error sending the data");

    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Error sending the data");
    delay(1000);
    lcd.clear();
    return false;
  }

  lcd.clear();
  lcd.setCursor(2, 1);
  lcd.print("Sent with success");
  lcd.setCursor(5, 2);
  lcd.print("Please Wait");
  delay(1000);
  lcd.clear();
  return true;
}

void navigation() {

  while (digitalRead(UP) == HIGH && digitalRead(DOWN) == HIGH && digitalRead(ENTER) == HIGH && digitalRead(BACK) == HIGH)
    ;

  lcd.clear();

  if (digitalRead(UP) == LOW) {

    if (menu == 0) {

      // main menu > select menu
      menu = 1;

    } else if (menu == 1) {

      // cycle select menu
      menuIndex = (menuIndex + 1) % mainMenuSize;

    } else if (menu == 2) {

      if (SelectedMenu == 0) {

        // Serial.println("section up");
        // cycle section menu
        sectionMenuIndex = (sectionMenuIndex + 1) % sectionMenuSize;

      } else if (SelectedMenu == 1) {
        // Serial.println("teachre up");
        // cycle teacher menu
        teacherMenuIndex = (teacherMenuIndex + 1) % teacherMenuSize;
      }
    }
  } else if (digitalRead(DOWN) == LOW) {

    if (menu == 0) {

      // main menu > select menu
      menu = 1;

      // cycle select menu
    } else if (menu == 1 && menuIndex >= 1) {

      menuIndex = (menuIndex - 1) % mainMenuSize;

    } else if (menu == 2) {

      if (SelectedMenu == 0 && sectionMenuIndex >= 1) {
        // Serial.println("section down");
        // cycle section menu
        sectionMenuIndex = (sectionMenuIndex - 1) % sectionMenuSize;

      } else if (SelectedMenu == 1 && teacherMenuIndex >= 1) {
        // Serial.println("teacher down");
        // cycle teacher menu
        teacherMenuIndex = (teacherMenuIndex - 1) % teacherMenuSize;
      }
    }
  } else if (digitalRead(ENTER) == LOW) {

    if (menu == 0) {

      // main menu > select menu
      menu = 1;

    } else if (menu == 1) {

      if (menuIndex == 2) {

        SelectedMenu = 1;

      } else {

        SelectedMenu = menuIndex;
      }

      menu = 2;

    } else if (menu == 2) {

      if (SelectedMenu == 0) {

        selectedSection = sectionMenuIndex;

        SelectedMenu = 1;

      } else if (SelectedMenu == 1) {

        selectedTeacher = teacherMenuIndex;

        menu = 3;
      }
    } else if (menu == 3) {

      sendData();
    }
  } else if (digitalRead(BACK) == LOW) {

    if (menu == 0) {

      // main menu > select menu
      menu = 1;

    } else if (menu == 1) {

      SelectedMenu = 0;

      menu = 0;

    } else if (menu == 2) {

      if (SelectedMenu == 0) {

        menu = 1;

      } else if (SelectedMenu == 1) {

        SelectedMenu = 0;
      }

    } else if (menu == 3) {

      menu = 2;
    }
  }

  delay(100);

  while (digitalRead(UP) == LOW || digitalRead(DOWN) == LOW || digitalRead(ENTER) == LOW || digitalRead(BACK) == LOW)
    ;
}


void display() {
  if (menu == 0) {

    // lcd.clear();
    lcd.setCursor(6, 0);
    lcd.print("Welcome!");
    lcd.setCursor(2, 1);
    lcd.print("Press any button");
    lcd.setCursor(1, 3);
    lcd.print("Research by ST12P3");

  } else if (menu == 1) {

    // lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Who are you?");
    lcd.setCursor(4, 2);
    lcd.print("> " + (String)mainMenuItems[menuIndex]);
    lcd.setCursor(15, 2);
    lcd.print("<");

  } else if (menu == 2) {

    if (SelectedMenu == 0) {
      // lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("Hello " + (String)mainMenuItems[menuIndex]);
      lcd.setCursor(2, 2);
      lcd.print(">    " + (String)sectionMenuItems[sectionMenuIndex]);
      lcd.setCursor(17, 2);
      lcd.print("<");
      lcd.setCursor((20 - ((String)teacherMenuItems[teacherMenuIndex]).length()) / 2, 3);
      lcd.print((String)teacherMenuItems[teacherMenuIndex]);

    } else if (SelectedMenu == 1) {

      // lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("Hello " + (String)mainMenuItems[menuIndex]);

      //Check if it is a visitor
      if (menuIndex != 2) {
        lcd.setCursor(7, 2);
        lcd.print((String)sectionMenuItems[sectionMenuIndex]);
      }

      lcd.setCursor(2, 3);
      lcd.print(">");
      lcd.setCursor((20 - ((String)teacherMenuItems[teacherMenuIndex]).length()) / 2, 3);
      lcd.print((String)teacherMenuItems[teacherMenuIndex]);
      lcd.setCursor(17, 3);
      lcd.print("<");
    }

  } else if (menu == 3) {

    // lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("CHECK IF AVIALABLE");

    lcd.setCursor(4, 1);
    lcd.print("-> " + (String)teacherMenuItems[teacherMenuIndex]);
    lcd.setCursor(4, 2);
    lcd.print("-> " + (String)sectionMenuItems[sectionMenuIndex]);

    lcd.setCursor(0, 3);
    lcd.print("   YES         NO");

  } else if (menu == 4) {
    String msg, subMsg;

    switch (response) {
      case 0:
        msg = "Waiting for";
        subMsg = "Response";
        break;
      case 1:
        digitalWrite(GREEN, HIGH);
        msg = "Available";
        subMsg = "Please come in";
        break;
      case 2:
        digitalWrite(ORANGE, HIGH);
        msg = "Busy";
        subMsg = "Please Wait";
        break;
      case 3:
        digitalWrite(RED, HIGH);
        msg = "Not Around";
        subMsg = "Come back later";
        break;
      default:
        break;
    };

    // lcd.clear();
    lcd.setCursor((20 - ((String)teacherMenuItems[teacherMenuIndex]).length()) / 2, 0);
    lcd.print((String)teacherMenuItems[teacherMenuIndex]);
    lcd.setCursor((17- msg.length()) / 2, 1);
    lcd.print("> " + msg + " <");
    lcd.setCursor((21 - subMsg.length()) / 2, 3);
    lcd.print(subMsg);

    delay(10000);

    //turn off led
    digitalWrite(GREEN, LOW);
    digitalWrite(ORANGE, LOW);
    digitalWrite(RED, LOW);

    lcd.clear();
    menu = 0;
    menuIndex = 0;
    SelectedMenu = 0;
    response = 0;
    display();

  } else if (menu == 5) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WAITING FOR RESPONSE");
    lcd.setCursor(2, 2);
    lcd.print("PLEASE WAIT");


    delay(5000);
    response = 0;
    menu = 0;

    // display();
  }
}