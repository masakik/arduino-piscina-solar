// Programa : Display LCD 16x2 e modulo I2C
// Autor : Arduino e Cia

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Inicializa o display no endereco 0x3F
//LiquidCrystal_I2C lcd(0x3F,16,2);
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
bool bl = true;
void setup()
{
  lcd.begin (16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Piscina");
  lcd.setCursor(0, 1);
  lcd.print("Versao 1.0");
  delay(3000);
}

void loop()
{

  lcd.setBacklight(HIGH);
  lcd.setCursor(0, 0);
  lcd.print("T1:45.6  T2:23.1C");
  lcd.setCursor(15, 1);
  if (bl) {
    lcd.print("*");
    bl = !bl;
  } else {
    lcd.print(" ");
    bl = !bl;
  }

  lcd.setCursor(0, 1);
  lcd.print("B1:OF R1:2h34m");
  delay(500);
  //lcd.setBacklight(LOW);
  //delay(500);
}
