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

#define sw_serial_rx_pin 8 //  Connect this pin to TX on the esp8266
#define sw_serial_tx_pin 9 //  Connect this pin to RX on the esp8266
#define esp8266_reset_pin 10 // Connect this pin to CH_PD on the esp8266, not reset. (let reset be unconnected)

// relogio sda/scl
// pino a4 e a5

#define LEDR 11
#define LEDG 12
#define LEDB 13

int v = 1; //versao

// parametros ajustaveis
unsigned int f1 = 32; //temperatura desejada na piscina. Depois a bomba nao funciona
unsigned int f2 = 4;  //diferencial t1-t2 para ligar a bomba
unsigned int f3 = 3; //diferencial t1-t2 para desligar a bomba

unsigned int f4 = 8; //temperatura em s1 para nao congelar. Histerese de 2 graus
// opera se t1 < 15 graus. Entao o maximo seria f4 = 13 mas cuidado!!



// ---------------------------------------------------------------

DeviceAddress s1; // s1 end do sensor da placa
DeviceAddress s2; // s2 end do sensor da piscina

// Define uma instancia do oneWire para comunicacao com o sensor
OneWire oneWire1(ONE_WIRE_BUS1);
OneWire oneWire2(ONE_WIRE_BUS2);

// Indica onde procurar os sensores de temperatura
DallasTemperature sensor1(&oneWire1);
DallasTemperature sensor2(&oneWire2);

// parametros internos
bool b1 = 0; //bomba da placa de aquecimento
bool b2 = 0; //bomba do filtro
unsigned long b1_ton = 0; // timer de tempo ligado
unsigned int b1_tempo = 0; // tempo acumulado de b1
unsigned int b1_1 = 60; // protecao: tempo máximo que b1 pode ficar ligado.
unsigned int b1_2 = 60; // protecao: tempo mínimmo que b1 fica desligado antes de ligar de novo

// o erro serial bom usar um esquema de bits, onde cada bit setado seria um tipo de erro
// em decimal seria a somatoria que daria um resultado interpretavel
int erro = 0; //erro geral: 0 sem erros, 1 erro de sensor1, 2 erro sensor2, outros erros
int info = 0; // flag de informacao: 4 - atingiu f4, 1 - atingiu f1, o que mais?
int p = 0;

float t1, t2; // temperaturas dos sensores t1-placa t2 piscina

TimerM timer1;
TimerM tmr_config, tmr_var;

void setup(void)
{
  Serial.begin(9600);
  setPins();
  StatusLed();
  Sensor('i');
  Config('p');
}

void loop()
{
  if (timer1.run(5)) { // tempo em segundos
    Sensor('i'); // inicializa
    Sensor('r'); // le
    StatusLed();
    if (erro != 0) Bomba1('o'); // desliga a bomba se tiver erro
    if (erro == 0 ) {
      if (t2 < f1) {
        Bomba1('d'); // modo diferencial se maximo nao atingido
        info = 0;
      } else {
        info = 1;
      }
    }
    if (erro == 0 && t1 < 15) Bomba1('a'); // anticongelamento
    Bomba1_tempo();
  }

  if (tmr_var.run(10)) {
    Variaveis('p'); // Mostra dados no serial monitor
  }

  if (tmr_config.run(60)) {
    Sensor('i');
    Config('p'); // mostra configuracao
  }
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
  pinMode(LEDB, OUTPUT);
  digitalWrite(LEDB, 1);
}


void Sensor(char action) {
  if (action == 'i') { // inicializa sensores
    sensor1.begin();
    sensor2.begin();
    if (sensor1.getDeviceCount() == 1 && sensor2.getDeviceCount() == 1) {
      erro = 0;
      sensor1.getAddress(s1, 0);
      sensor2.getAddress(s2, 0);
    } else if (sensor1.getDeviceCount() == 0) {
      erro = 1;
    }
    else if (sensor2.getDeviceCount() == 0) {
      erro = 2;
    }
    LogError();

  } else if (action == 'r') { // le sensores somente se estiver OK
    if (erro == 0) {
      sensor1.requestTemperatures();
      sensor2.requestTemperatures();
      t1 = sensor1.getTempC(s1);
      t2 = sensor2.getTempC(s2);
      if (t1 < -10 || t2 < -10) { // se a temperatura estiver fora da faixa e porque houve erro
        erro = 3;
      }
    }
  }
  LogError();
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
    Serial.print(",t1:"); Serial.print(t1,1);
    Serial.print(",t2:"); Serial.print(t2,1);
    Serial.print(",b1:"); Serial.print(b1);
    Serial.print(",b2:"); Serial.print(b2);
    Serial.print(",b1_tempo:"); Serial.print(b1_tempo);
    Serial.println("}");
  }
}

void Bomba1(char action) { // muda o estado da bomba1 se necessário
  if (action == 'd') {// operacao diferencial
    if (t1 - t2 > f2) { // liga
      b1 = 1; digitalWrite(B1_PIN, !b1); // !b1 pois o modulo de rele é invertido
    }
    else if (t1 - t2 < f3) { // desliga
      b1 = 0; digitalWrite(B1_PIN, !b1); // !b1 pois o modulo de rele é invertido
    }
  }
  else if (action == 'a') { // operacao anticongelamento
    if (t1 < f4) {
      b1 = 1; digitalWrite(B1_PIN, !b1); // !b1 pois o modulo de rele é invertido
      info = 4;
    } else if (t1 > f4 + 2) {
      b1 = 0; digitalWrite(B1_PIN, !b1); // !b1 pois o modulo de rele é invertido
      info = 0;
    }
  }
  else if (action == 'o') {
    b1 = 0; digitalWrite(B1_PIN, !b1);
  }
}

void Bomba1_tempo() {// acumula o tempo ligado
  if (b1 == 1) {
    b1_tempo += (millis() - b1_ton) / 1000;
    b1_ton = millis();
  } else {
    b1_ton = millis();
  }
}

void StatusLed() {
  if (erro == 0) {
    digitalWrite(LEDG, 0);
    digitalWrite(LEDR, 1);
  } else {
    digitalWrite(LEDR, 0);
    digitalWrite(LEDG, 1);
  }
}

void LogError() {
  // grava log de erros quando necessario
}

