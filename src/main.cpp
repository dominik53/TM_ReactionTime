#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define TEST_COUNT 5        // per 1 sense, min=1 max=33
#define TEST_MAX_TIME 10000 // ms, max=99999

#define BUTT_1 9
#define REACT_BUTT_1 10 // sight
#define REACT_BUTT_2 1  // hearing
#define REACT_BUTT_3 0  // touch
#define LED 7
#define BUZZER 3
#define VIBRATOR 4
#define SDA_PIN 5
#define SCL_PIN 6
#define FREE_ANALOG_PIN 2

TaskHandle_t task1;
TaskHandle_t task2;
LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7,3,POSITIVE);

// LCD
int currentScreen=0;
uint8_t customCharA[8] = {0,14,1,15,17,15,2,1};       // ą
uint8_t customCharC[8] = {2,4,14,16,16,17,14,0};      // ć
uint8_t customCharL[8] = {12,4,6,4,12,4,14,0};        // ł
uint8_t customCharZ[8] = {4,0,31,2,4,8,31,0};         // ż
uint8_t customCharS[8] = {2,4,15,8,14,1,30,0};        // ś
uint8_t customCharArrowRight[8] = {0,4,2,31,2,4,0,0}; // arrow right

// test session
uint8_t testQueue[TEST_COUNT*3];
int currentTest=1;
int reactionTime1[TEST_COUNT];  // ms
int reactionTime2[TEST_COUNT];  // ms
int reactionTime3[TEST_COUNT];  // ms
int meanReaction[3];            // ms

// functions
void scanI2C(void *parameter);        // dev only - show active i2c addresses
void performTests(void *parameter);   // handle tests
void buttonHandle(void *parameter);   // handles control button
void displayScreen(int screenIndex);  // display screen on LCD
void restartSession();                // restart test session
int fillQueue();                      // fill tests queue with 1, 2, 3

void setup() {
  Serial.begin(115200);

  randomSeed(analogRead(FREE_ANALOG_PIN));

  pinMode(BUTT_1, INPUT_PULLUP);
  pinMode(REACT_BUTT_1, INPUT_PULLUP);
  pinMode(REACT_BUTT_2, INPUT_PULLUP);
  pinMode(REACT_BUTT_3, INPUT_PULLUP);
  
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
  pinMode(VIBRATOR, OUTPUT);
  digitalWrite(VIBRATOR, LOW);
  
  Wire.begin(SDA_PIN,SCL_PIN);
  lcd.begin(20, 4);

  lcd.createChar(0, customCharA); // location 7 reserved for dynamic byte definition
  lcd.createChar(1, customCharC);
  lcd.createChar(2, customCharL);
  lcd.createChar(3, customCharZ);
  lcd.createChar(4, customCharS);
  lcd.createChar(5, customCharArrowRight);
  
  // delay(10000); // a więc coś poszło nie tak :(
  xTaskCreatePinnedToCore(buttonHandle,"buttonHandle",1024,nullptr,1,&task1,1 );

  displayScreen(currentScreen++); // project info
  displayScreen(currentScreen);   // home
}

void loop() {

}
///////////////////


void buttonHandle(void *parameter){
  for(;;){
    if(digitalRead(BUTT_1)==LOW){
      delay(50);

      switch(currentScreen){
        case 1:{
          currentScreen=2;
          xTaskCreatePinnedToCore(performTests,"performTests",4096,nullptr,1,&task2,1 );

          break;
        }
        case 2:{
          digitalWrite(LED, LOW);
          digitalWrite(BUZZER, LOW);
          digitalWrite(VIBRATOR, LOW);
          
          vTaskDelete(task2);
          currentScreen=1;
          displayScreen(currentScreen);
          break;
        }
      
        default:{
          currentScreen=1;
          displayScreen(currentScreen);
          break;
        }
      }
    
      while (digitalRead(BUTT_1) == LOW) {
        delay(50);
      }
    }else{
      delay(20);
    }
  }
  vTaskDelete(NULL);
}

void performTests(void *parameter){
  restartSession();

  if(fillQueue()){
    Serial.println("Error when populating queue");
    vTaskDelete(NULL);
  }

  bool BUTT1 = false;
  bool BUTT2 = false;
  bool BUTT3 = false;
  unsigned long start = 0;
  int A = 0;
  int B = 0;
  int C = 0;

  unsigned long reactiontime = 0;

  for (int i = 0; i < (TEST_COUNT * 3); i++){
    displayScreen(currentScreen);
    reactiontime = 0;
    BUTT1 = false;
    BUTT2 = false;
    BUTT3 = false;

    delay(random(2000,4001)); // random delay from 2 to 4 sec
    while (digitalRead(REACT_BUTT_1) == LOW || digitalRead(REACT_BUTT_2) == LOW || digitalRead(REACT_BUTT_3) == LOW){
      delay(random(2000,4001)); // random delay from 2 to 4 sec
    }

    if(testQueue[i]==1){
      digitalWrite(LED, HIGH);
    }else if(testQueue[i]==2){
      digitalWrite(BUZZER, HIGH);
    }else if(testQueue[i]==3){
      digitalWrite(VIBRATOR, HIGH);
    }else{
      Serial.println("testQueue value not expected.");
      vTaskDelete(NULL);
    }

    start = millis();

    while (reactiontime < TEST_MAX_TIME){
      if (digitalRead(REACT_BUTT_1) == LOW){
        reactiontime = millis() - start;
        delay(50);
        BUTT1 = true;
      }
      if (digitalRead(REACT_BUTT_2) == LOW){
        reactiontime = millis() - start;
        delay(50);
        BUTT2 = true;
      }
      if (digitalRead(REACT_BUTT_3) == LOW){
        reactiontime = millis() - start;
        delay(50);
        BUTT3 = true;
      }

      if(BUTT1 || BUTT2 || BUTT3){
        if ((BUTT1 && !BUTT2 && !BUTT3) || (!BUTT1 && BUTT2 && !BUTT3) || (!BUTT1 && !BUTT2 && BUTT3)){ // if only 1 button pressed
          if(testQueue[i]==1 && BUTT1){
            reactionTime1[A++] = reactiontime;
            break;
          }else if(testQueue[i]==2 && BUTT2){
            reactionTime2[B++] = reactiontime;
            break;
          }else if(testQueue[i]==3 && BUTT3){
            reactionTime3[C++] = reactiontime;
            break;
          }else{
            BUTT1 = false;
            BUTT2 = false;
            BUTT3 = false;
            Serial.println("Wrong button pressed.");
            continue;
          }

          break;
        }else{ // if more than 1 button pressed
          BUTT1 = false;
          BUTT2 = false;
          BUTT3 = false;
          Serial.println("More than 1 button pressed.");
          delay(50);
        }
      }else{ // if no buttons pressed
        reactiontime = millis() - start;
      }
      
    }

    if(testQueue[i]==1){
      digitalWrite(LED, LOW);
    }else if(testQueue[i]==2){
      digitalWrite(BUZZER, LOW);
    }else if(testQueue[i]==3){
      digitalWrite(VIBRATOR, LOW);
    }else{
      Serial.println("testQueue value not expected.");
      vTaskDelete(NULL);
    }

    if (reactiontime >= TEST_MAX_TIME){ // didnt react at all
      currentScreen=1;
      displayScreen(currentScreen);

      vTaskDelete(NULL);
    }

    currentTest++;
  }

  long temp1=0,temp2=0,temp3=0;
  for(int i=0;i<TEST_COUNT;i++){
    temp1+=reactionTime1[i];
    temp2+=reactionTime2[i];
    temp3+=reactionTime3[i];
  }
  meanReaction[0]=(int)round(temp1/TEST_COUNT);
  meanReaction[1]=(int)round(temp2/TEST_COUNT);
  meanReaction[2]=(int)round(temp3/TEST_COUNT);

  currentScreen=3;
  displayScreen(currentScreen);

  vTaskDelete(NULL);
}

void restartSession(){
  currentTest=1;
  for(int i=0;i<TEST_COUNT*3;i++){
    testQueue[i]=0;
  }
  for(int i=0;i<TEST_COUNT;i++){
    reactionTime1[i]=0;
    reactionTime2[i]=0;
    reactionTime3[i]=0;
  }
  meanReaction[0]=0;
  meanReaction[1]=0;
  meanReaction[2]=0;

  return;
}

int fillQueue() {
  int x=0;
  bool infinite=false;

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < TEST_COUNT; j++) {
      infinite=false;
      x = random(3 * TEST_COUNT - 1);
      while (testQueue[x] != 0) {
        if (++x > (3 * TEST_COUNT - 1)){
          x = 0;
          if(infinite){
            Serial.println("Infinite loop in fillQueue");
          }else{
            infinite=true;
          }
        }
      }
      testQueue[x] = i + 1;
    }
  }

  return 0;
}

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
    case 2: // in progress
    {
      char LCDcontent[4][21];
        // "  Testy w toku...   ",
        // "       xx/xx        ",
        // "                    ",
        // "                    "

      String tempVariable="       ";
      if(currentTest>9){
        tempVariable += String(currentTest) + "/";
      }else{
        tempVariable += "0" + String(currentTest) + "/";
      }
      if(TEST_COUNT*3>9){
        tempVariable += String(TEST_COUNT*3) + "        ";
      }else{
        tempVariable += "0" + String(TEST_COUNT*3) + "        ";
      }

      strcpy(LCDcontent[0], "  Testy w toku...   ");
      strcpy(LCDcontent[1], tempVariable.c_str());
      strcpy(LCDcontent[2], "                    ");
      strcpy(LCDcontent[3], "                    ");
      
      for(int i=0;i<4;i++){
        lcd.setCursor(0, i);
        for(int j=0;j<20;j++){
          lcd.print(LCDcontent[i][j]);
        }
      }

      break;
    }
    case 3: // show results
    {
      char LCDcontent[4][21];
        // "Czas reakcji:       ",
        // "wzrok p xxxxx ms    ",
        // "słuch p xxxxx ms    ",
        // "dotyk p xxxxx ms    "

      String tempVariable1="wzrok p ";
      if(meanReaction[0]>99999){
        tempVariable1 += "xxxxx ms    ";
      }else if(meanReaction[0]>9999){
        tempVariable1 += String(meanReaction[0]) + " ms    ";
      }else if(meanReaction[0]>999){
        tempVariable1 += "0" + String(meanReaction[0]) + " ms    ";
      }else if(meanReaction[0]>99){
        tempVariable1 += "00" + String(meanReaction[0]) + " ms    ";
      }else if(meanReaction[0]>9){
        tempVariable1 += "000" + String(meanReaction[0]) + " ms    ";
      }else{
        tempVariable1 += "0000" + String(meanReaction[0]) + " ms    ";
      }
      
      String tempVariable2="sluch p ";
      if(meanReaction[1]>99999){
        tempVariable2 += "xxxxx ms    ";
      }else if(meanReaction[1]>9999){
        tempVariable2 += String(meanReaction[1]) + " ms    ";
      }else if(meanReaction[1]>999){
        tempVariable2 += "0" + String(meanReaction[1]) + " ms    ";
      }else if(meanReaction[1]>99){
        tempVariable2 += "00" + String(meanReaction[1]) + " ms    ";
      }else if(meanReaction[1]>9){
        tempVariable2 += "000" + String(meanReaction[1]) + " ms    ";
      }else{
        tempVariable2 += "0000" + String(meanReaction[1]) + " ms    ";
      }

      String tempVariable3="dotyk p ";
      if(meanReaction[2]>99999){
        tempVariable3 += "xxxxx ms    ";
      }else if(meanReaction[2]>9999){
        tempVariable3 += String(meanReaction[2]) + " ms    ";
      }else if(meanReaction[2]>999){
        tempVariable3 += "0" + String(meanReaction[2]) + " ms    ";
      }else if(meanReaction[2]>99){
        tempVariable3 += "00" + String(meanReaction[2]) + " ms    ";
      }else if(meanReaction[2]>9){
        tempVariable3 += "000" + String(meanReaction[2]) + " ms    ";
      }else{
        tempVariable3 += "0000" + String(meanReaction[2]) + " ms    ";
      }

      strcpy(LCDcontent[0], "Czas reakcji:       ");
      strcpy(LCDcontent[1], tempVariable1.c_str());
      strcpy(LCDcontent[2], tempVariable2.c_str());
      strcpy(LCDcontent[3], tempVariable3.c_str());
      
      for(int i=0;i<4;i++){
        lcd.setCursor(0, i);
        for(int j=0;j<20;j++){
          if(i==2 && j==1){ // ł
            lcd.write(byte(2));
          }else if((i==1 && j==6)||(i==2 && j==6)||(i==3 && j==6)){ // arrows
            lcd.write(byte(5));
          }else{
            lcd.print(LCDcontent[i][j]);
          }
        }
      }

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
