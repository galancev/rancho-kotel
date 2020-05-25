// Пины подключения двухстрочного LCD
#define LCD_RS_RIN 4
#define LCD_E_PIN 5
#define LCD_DB4_PIN 9
#define LCD_DB5_PIN 10
#define LCD_DB6_PIN 11
#define LCD_DB7_PIN 12

#define STATE_MEM 0 // Ячейка памяти для сохранения требуемой температуры

#define ENC_PIN_A 3 // Пин состояния енкодера
#define ENC_PIN_B 8 // Пин состояния енкодера

#define RELAY_PIN 7 // Управляющий пин реле

#define DHT_PIN 2 // Пин датчика температуры
#define DHTTYPE DHT22

#define DET_TEMP 2 // При снижении на сколько десятых градуса включать обогрев

#define TICK_TEMP 1024 // Частота опроса температурного датчика
#define TICK_ENC 5

#include <DHT.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <looper.h>

LiquidCrystal lcd(LCD_RS_RIN, LCD_E_PIN, LCD_DB4_PIN, LCD_DB5_PIN, LCD_DB6_PIN, LCD_DB7_PIN);

DHT dht(DHT_PIN, DHTTYPE);

looper job; //create a new istance of the class leOS

int hum = 0;
int tem = 0;

boolean isHeat = true;

int needTemp = 48;

int encoder0PinALast = LOW;

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  
  pinMode(ENC_PIN_A, INPUT);
  pinMode(ENC_PIN_B, INPUT);
  
  lcd.begin(16, 2);
  lcd.noBlink();
  lcd.noCursor();
  lcd.print("System ready!");
  
  dht.begin();
    
  needTemp = EEPROM.read(STATE_MEM);
  
  job.addJob(checkTemp, TICK_TEMP);
  job.addJob(checkEncoder, TICK_ENC);
  
//  Serial.begin(9600);
}

void loop() {
  job.scheduler();
}
void checkTemp() {
  int h = dht.readHumidity();
  int t = dht.readTemperature() * 10;
  
  if (isnan(t) || isnan(h)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Fail");
  } else {
    if(t != tem || h != hum) {
      tem = t;
      hum = h;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("T: ");
      lcd.print(float(t)/10, 1);
      lcd.print("C  H: ");
      lcd.print(h);
      lcd.print("%");
      lcd.setCursor(0, 1);
      lcd.print("=> ");
      lcd.print(float(needTemp)/2, 1);
      lcd.print("C");
      if(isHeat) {
        lcd.setCursor(12, 1);
        lcd.print("HEAT");
      } else {
        lcd.setCursor(12, 1);
        lcd.print("    ");          
      }
    }
  }
  
  if(isHeat) {
    if (tem >= needTemp*5) {
      digitalWrite(RELAY_PIN, 1);
      lcd.setCursor(12, 1);
      lcd.print("    ");
      isHeat = false;
    }
  } else {
    if(tem <= needTemp*5 - DET_TEMP) {
      digitalWrite(RELAY_PIN, 0);
      lcd.setCursor(12, 1);
      lcd.print("HEAT");
      isHeat = true;
    }
  }
}

void checkEncoder() {
  int enc_a = digitalRead(ENC_PIN_A);

  //Serial.print(enc_a);
//  Serial.print(" ");
//  Serial.println(digitalRead(ENC_PIN_B));

  if((encoder0PinALast == LOW) && (enc_a == HIGH)) {
    if(digitalRead(ENC_PIN_B)) {
      needTemp = needTemp + 1;
    } else {
      needTemp = needTemp - 1;
    }
    if(needTemp < 10*2) needTemp = 10*2;
    if(needTemp > 35*2) needTemp = 35*2;
    
    EEPROM.write(STATE_MEM, needTemp);
    
    lcd.setCursor(3, 1);
    lcd.print(float(needTemp)/2, 1);
  }
  encoder0PinALast = enc_a;
}

