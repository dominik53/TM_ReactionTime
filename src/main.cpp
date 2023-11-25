#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define BUTT_1 9
#define REACT_BUTT_1 10 // wzrok
#define REACT_BUTT_2 1  // słuch
#define REACT_BUTT_3 0  // dotyk
#define LED 7
#define BUZZER 3
#define VIBRATOR 4
#define SDA_PIN 5
#define SCL_PIN 6

TaskHandle_t task1;
LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7,3,POSITIVE);

// LCD
int currentScreen=0;
uint8_t customCharA[8] = {0,14,1,15,17,15,2,1};       // ą
uint8_t customCharC[8] = {2,4,14,16,16,17,14,0};      // ć
uint8_t customCharL[8] = {12,4,6,4,12,4,14,0};        // ł
uint8_t customCharZ[8] = {4,0,31,2,4,8,31,0};         // ż
uint8_t customCharS[8] = {2,4,15,8,14,1,30,0};        // ś
uint8_t customCharArrowRight[8] = {0,4,2,31,2,4,0,0}; // arrow right

// functions
void scanI2C(void *parameter);        // dev only - show active i2c addresses
void displayScreen(int screenIndex);  // display screen on LCD

void setup() {
  Serial.begin(115200);

  pinMode(BUTT_1, INPUT_PULLUP);
  pinMode(REACT_BUTT_1, INPUT_PULLUP);
  pinMode(REACT_BUTT_2, INPUT_PULLUP);
  pinMode(REACT_BUTT_3, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(VIBRATOR, OUTPUT);

  digitalWrite(LED, LOW);
  digitalWrite(BUZZER, LOW);
  digitalWrite(VIBRATOR, LOW);

  Wire.begin(SDA_PIN,SCL_PIN);
  lcd.begin(20, 4);

  lcd.createChar(0, customCharA); // location 7 reserved for dynamic byte definition
  lcd.createChar(1, customCharC);
  lcd.createChar(2, customCharL);
  lcd.createChar(3, customCharZ);
  lcd.createChar(4, customCharS);
  lcd.createChar(5, customCharArrowRight);
  
  // delay(5000);
  // xTaskCreatePinnedToCore(serverConfig,"serverConfig",2048,nullptr,1,&task1,1 );

  displayScreen(currentScreen++); // project info
  displayScreen(currentScreen);   // home
}

void loop() {
  if(digitalRead(BUTT_1)==LOW){
    delay(50);

    switch(currentScreen){
      case 2:{ // przyklad: wcisniemy przycisk i jestesmy na ekranie przeprowadzania testu
        // operacja
        break;
      }
      
      default:
          break;
    }
    
    while (digitalRead(BUTT_1) == LOW) {
      delay(50);
    }
  }


  if(digitalRead(REACT_BUTT_1)==LOW){
    delay(50);

    switch(currentScreen){
      case 2:{
        
        break;
      }
      
      default:
          break;
    }

    while (digitalRead(REACT_BUTT_1) == LOW) {
      delay(50);
    }
  }


  if(digitalRead(REACT_BUTT_2)==LOW){
    delay(50);

    switch(currentScreen){
      case 2:{
        
        break;
      }
      
      default:
          break;
    }

    while (digitalRead(REACT_BUTT_2) == LOW) {
      delay(50);
    }
  }


  if(digitalRead(REACT_BUTT_3)==LOW){
    delay(50);
    
    switch(currentScreen){
      case 2:{
        
        break;
      }
      
      default:
          break;
    }
    
    while (digitalRead(REACT_BUTT_3) == LOW) {
      delay(50);
    }
  }


}
///////////////////


void displayScreen(int screenIndex){
  lcd.clear();
  lcd.noBlink();
  lcd.setCursor(0, 0);
  
  switch (screenIndex) {
    case 0: // title screen
    {
      char LCDcontent[4][21] = {
        "Pomiar czasu reakcji",
        "                    ",
        "   Dominik Kijak    ",
        "  Jakub Wisniewski  "
      };

      for(int i=0;i<4;i++){
        for(int j=0;j<20;j++){
          if(i==3 && j==10){ // ś
            lcd.write(byte(4));
          }else{
            lcd.print(LCDcontent[i][j]);
          }
        }
      }

      delay(2000);
      lcd.setCursor(0,0);
      for(int i=0;i<20;i++){ // animacja czyszczenia ekranu
        for(int j=0;j<4;j++){
          lcd.setCursor(i,j);
          lcd.print(" ");
        }
        delay(20);
      }

      break;
    }
    case 1: // home
    {
      char LCDcontent[4][21] = {
        " Rozpocznij pomiary ",
        "                    ",
        "                    ",
        "        pOK         "
      };
      
      for(int i=0;i<4;i++){
        lcd.setCursor(0, i);
        for(int j=0;j<20;j++){
          if(i==3 && j==8){ // strzalka w prawo
            lcd.write(byte(5));
          }else{
            lcd.print(LCDcontent[i][j]);
          }
        }
      }

      lcd.setCursor(8,3);
      lcd.blink();

      break;
    }


    default:
      break;
  }
}

/////////////////////////////////

// Find I2C devices
void scanI2C(void *parameter){
  for(;;){
    byte error, address;
    int nDevices;
  
    Serial.println("Scanning...");
    nDevices = 0;
    for(address = 1; address < 127; address++ )
    {
      Wire.beginTransmission(address);
      error = Wire.endTransmission();
  
      if (error == 0)
      {
        Serial.print("I2C device found at address 0x");
        if (address<16)
          Serial.print("0");
        Serial.print(address,HEX);
        Serial.println("  !");
  
        nDevices++;
      }
      else if (error==4)
      {
        Serial.print("Unknown error at address 0x");
        if (address<16)
          Serial.print("0");
        Serial.println(address,HEX);
      }    
    }
    if (nDevices == 0)
      Serial.println("No I2C devices found\n");
    else
      Serial.println("done\n");
  
    delay(5000);
  }
  vTaskDelete(NULL);
}