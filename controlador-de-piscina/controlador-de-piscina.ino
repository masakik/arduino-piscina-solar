//Programa controloador de piscina
//com aquecimento e display

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include <TimerM.h>

// Porta do pino de sinal do DS18B20
#define ONE_WIRE_BUS1 2
#define ONE_WIRE_BUS2 3
#define B1_PIN 4 //pino do rele da bomba1
#define B2_PIN 5 //pino do rele da bomba2
#define B3_PIN 6 // rele3 opcional
#define B4_PIN 7 // rele4 opcional

#define LEDR 8
#define LEDG 9

#define sw_serial_rx_pin 10 //  Connect this pin to TX on the esp8266
#define sw_serial_tx_pin 11 //  Connect this pin to RX on the esp8266
#define esp8266_reset_pin 12 // Connect this pin to CH_PD on the esp8266, not reset. (let reset be unconnected)

#define BTN_PIN A0 // por enquanto somente um botao

// relogio sda/scl, display 16x2
// pino a4 e a5

int v = 1; //versao

// parametros ajustaveis
//unsigned int f1 = 32; //temperatura desejada na piscina. Acima a bomba1 nao funciona
//unsigned int f2 = 4; //diferencial t1-t2 para ligar a bomba
//unsigned int f3 = 3; //diferencial t1-t2 para desligar a bomba
//unsigned int f4 = 15; //temperatura em s1 para nao congelar. Histerese em f5
//unsigned int f5 = 40; //tempo de anticongelamento

unsigned int f1 = 32; //temperatura desejada na piscina. Acima a bomba1 nao funciona
unsigned int f2 = 8; //diferencial t1-t2 para ligar a bomba
unsigned int f3 = 4; //diferencial t1-t2 para desligar a bomba
unsigned int f4 = 8; //temperatura em s1 para nao congelar. Histerese em f5
unsigned int f5 = 40; //tempo de anticongelamento
// ---------------------------------------------------------------

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
int lcd_active_tmr = 60; // tempo para desligamento
bool lcd_active = 1; // se lcd estiver aceso

DeviceAddress s1; // s1 end do sensor da placa
DeviceAddress s2; // s2 end do sensor da piscina

// Define uma instancia do oneWire para comunicacao com o sensor
OneWire oneWire1(ONE_WIRE_BUS1);
OneWire oneWire2(ONE_WIRE_BUS2);

// Indica onde procurar os sensores de temperatura
DallasTemperature sensor1(&oneWire1);
DallasTemperature sensor2(&oneWire2);

// parametros internos
bool b1 = 0, b2 = 0; //estado da bomba1 (aquecimento) e bomba2 (filtro)
unsigned long r1 = 0, r1_tmp = 0, r2 = 0; // tempo acumulado de b1 e b2, auxiliares
//unsigned int b1_1 = 60; // protecao: tempo mÃ¡ximo que b1 pode ficar ligado.
//unsigned int b1_2 = 60; // protecao: tempo mÃ­nimmo que b1 fica desligado antes de ligar de novo

int erro = 0; //erro geral: 0 sem erros, 1 erro de sensor1, 2 erro sensor2, outros erros
int info = 0; // flag de informacao: 4 - atingiu f4,1- b1 ligado; 2 - atingiu f1, o que mais?
//int p = 0;

float t1, t2; // temperaturas dos sensores t1-placa t2 piscina

bool btn1, btn1_prev;
int btn1r, btn1r_prev;

TimerM tmr1, tmr2, tmr3, tmr4, tmr5, tmr6, tmr_btn, tmr7;

void setup(void)
{
  Serial.begin(9600);
  Serial.print("Controlador de piscina versão ");
  Serial.println(v);
  setPins();
  delay(5); // aguarda um momento para inicializar o millis()
  erro = Sensor('i'); // inicializa
  lcd_welcome(2000);
}

void loop()
{
  if (tmr6.run(2)) {

  }

  if (tmr3.run(60)) {
    Config('p'); // mostra configuracao
  }

  if (tmr1.run(3)) { // tempo em segundos
    if (erro == 0) erro = Sensor('r'); // le primeiro

    if (erro == 0) {
      if (t1 > f4 && !tmr5.countdown("status")) { //se acima da temp de congelamento
        if (t2 < f1) {// modo diferencial se maximo nao atingido
          if (t1 - t2 >= f2) { // liga
            Bomba1('l');
            info = 1;
          }
          else if (t1 - t2 <= f3) { // desliga
            Bomba1('o');
            info = 0;
          }
        } else { // se maximo, desliga a bomba
          Bomba1('o');
          info = 2;
        }
      } else { //se abaixo de congelamento
        Bomba1('l');
        info = 4;
        tmr5.countdown("start", f5);
      }
    } else {
      info = 0;
      Bomba1('o'); // desliga a bomba se tiver erro
    }
  }

  if (tmr5.countdown("finish")) { // pára anticongelamento
    if (t1 < f4) { // se abaixo do congelamento ainda
      tmr5.countdown("start", f5);
    } else {
      info = 0;
      Bomba1('o');
    }
  }

  if (tmr7.countdown("finish")) {
    lcd.setBacklight(LOW);
    lcd_active = 0;
    Serial.println("Desligando lcd ..");
  }

  if (tmr2.run(1)) {
    if (b1 == 1) { // computa o tempo ligado da bomba
      r1 += millis() - r1_tmp;
      r1_tmp = millis();
    } else {
      r1_tmp = millis();
    }

    if (erro != 0) { // verfifica sensores se necessário
      erro = Sensor('i');
    }
    StatusLed();
    Variaveis('d');
  }

  if (tmr4.run(30)) {
    Variaveis('p'); // Mostra dados no serial monitor
  }

  chk_buttons();
  // while (1) {}
}

void lcd_welcome(unsigned int tempo) {
  lcd.begin (16, 2);
  lcd.setBacklight(HIGH);
  lcd_active = 1;
  tmr7.countdown("restart", lcd_active_tmr);
  lcd.setCursor(0, 0);
  lcd.print("Piscina             ");
  lcd.setCursor(0, 1);
  lcd.print("Versao 1.0           ");
  delay(tempo);
}

void setPins() {
  pinMode(B1_PIN, OUTPUT);
  digitalWrite(B1_PIN, !b1);
  pinMode(B2_PIN, OUTPUT);
  digitalWrite(B2_PIN, !b2);

  pinMode(LEDR, OUTPUT);
  digitalWrite(LEDR, 1);
  pinMode(LEDG, OUTPUT);
  digitalWrite(LEDG, 1);
}

void chk_buttons() {

  btn1r = analogRead(BTN_PIN);

  btn1 = (btn1r > 500) ? 1 : 0;
  if (btn1 != btn1_prev) {
    if (btn1 == 1) {
      //Serial.println("btn press ");
    } else {
      Serial.println("btn release ");
      tmr7.countdown("restart", lcd_active_tmr);
      lcd.setBacklight(HIGH);
      if (!lcd_active) Serial.println("Ativando lcd ..");
      lcd_active = 1;
    }
    //delay(30);
  }
  btn1_prev = btn1;

}

int Sensor(char action) {
  if (action == 'i') { // inicializa sensores
    sensor1.begin();
    sensor2.begin();
    if (sensor1.getDeviceCount() == 1 && sensor2.getDeviceCount() == 1) {
      sensor1.getAddress(s1, 0);
      sensor2.getAddress(s2, 0);
      sensor1.setResolution(s1, 9);
      sensor2.setResolution(s2, 9);
      return 0;
    } else if (sensor1.getDeviceCount() == 0) {
      Serial.println("Erro sensor 1");
      return 1;
    }
    else if (sensor2.getDeviceCount() == 0) {
      Serial.println("Erro sensor 2");
      return 2;
    }
  }
  else if (action == 'r') { // le sensores
    sensor1.requestTemperatures();
    sensor2.requestTemperatures();
    t1 = sensor1.getTempC(s1);
    t2 = sensor2.getTempC(s2);
    if (t1 < -100 || t2 < -100) { // se a temperatura estiver fora da faixa e porque houve erro
      return 3;
    } else {
      return 0;
    }
  }
}

// imprime os parametros de configuracao
void Config(char action) {
  if (action == 'p') { // imprime
    Serial.print("{");
    Serial.print("v:"); Serial.print(v);
    Serial.print(",f1:"); Serial.print(f1);
    Serial.print(",f2:"); Serial.print(f2);
    Serial.print(",f3:"); Serial.print(f3);
    Serial.print(",f4:"); Serial.print(f4);
    Serial.print(",f5:"); Serial.print(f5);
    Serial.print(",btn1:"); Serial.print(btn1r);
    Serial.println("}");
  } else if (action == 's') { // salva
    // salva
  } else if (action == 'l') { // carrega
    // carrega
  } else if (action == 'r') { // reseta a configuracao para o padrao
    // reset
  }
}

void Variaveis(char action) {
  if (action == 'p') { // imprime dados
    Serial.print("{");
    Serial.print("e:"); Serial.print(erro);
    Serial.print(",i:"); Serial.print(info);
    Serial.print(",t1:"); Serial.print(t1, 1);
    Serial.print(",t2:"); Serial.print(t2, 1);
    Serial.print(",b1:"); Serial.print(b1);
    Serial.print(",r1:"); Serial.print(r1 / 1000);
    Serial.print(",b2:"); Serial.print(b2);
    Serial.println("}");
  } else if (action == 'd') {
    //lcd.setBacklight(HIGH);
    lcd.setCursor(0, 0);
    lcd.print("T1:");
    lcd.print(t1, 1);
    lcd.print("  T2:");
    lcd.print(t2, 1);
    lcd.setCursor(0, 1);
    lcd.print("B1:");
    if (b1 == 1) lcd.print("LIG    ");
    else lcd.print("DES    ");

    lcd.setCursor(7, 1);
    lcd.print("R1:");
    print_tempo(r1);
    lcd.print("         ");

  }
}

void print_tempo(unsigned long tempo) {  // tempo esta em millis

  int h = 0;
  int m = 0;
  int s = 0;
  int ms = 0;

  if (tempo > 3599999) {
    h = (((tempo / 1000L) / 60L) / 60L) % 24;
    if (h > 0) {
      if (h < 10) {
        lcd.print("0");
      }
      lcd.print(h);
      lcd.print("h");
    }
  }
  if (tempo > 59999) {
    m = ((tempo / 1000L) / 60L) % 60;
    if (m > 0) {
      if (m < 10) {
        lcd.print("0");
      }
      lcd.print(m);
      lcd.print("m");
    }
  }
  if (tempo < 3599999) {
    s = (tempo / 1000L) % 60;
    if (s < 10) {
      lcd.print("0");
    }
    lcd.print(s);
    lcd.print("s");
  }
  //  if (tempo < 59999) {
  //    ms = tempo % 1000L;
  //    strcat(ret, "s");
  //    strcat(ret, itoa(ms,tmp,10));
  //  }

  //  timeOut.toCharArray(ret, timeOut.length());
  //return ret;

  //timeOut = "";

}

void Bomba1(char action) { // muda o estado da bomba1 se necessÃ¡rio
  if (action == 'o') {
    if (b1 == 1) {
      Variaveis('p');
      Variaveis('d');
    }
    b1 = 0; digitalWrite(B1_PIN, !b1);
  }
  else if (action == 'l') {
    if (b1 == 0) {
      Variaveis('p');
      Variaveis('d');
    }
    b1 = 1; digitalWrite(B1_PIN, !b1);
  }
}

void StatusLed() {
  if (erro == 0) {
    digitalWrite(LEDR, 1);
    digitalWrite(LEDG, 0);
  } else {
    if (info == 0) {
      digitalWrite(LEDR, 0);
      digitalWrite(LEDG, 1);
    }
    else if (info == 1) { // b1 ligado
      //blink1(); 
    }
    else if (info == 2) // atingiu f1
      //blink2();
  }
}

