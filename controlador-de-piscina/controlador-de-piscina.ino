#include <OneWire.h>
#include <DallasTemperature.h>
#include <TimerM.h>

// Porta do pino de sinal do DS18B20
#define ONE_WIRE_BUS1 2
#define ONE_WIRE_BUS2 3
#define B1_PIN 4 //pino do rele da bomba1
#define B2_PIN 5 //pino do rele da bomba2

#define LEDR 11
#define LEDG 12
#define LEDB 13

int v = 1; //versao

// parametros ajustaveis
unsigned int f02 = 4;  //diferencial t1-t2 para ligar a bomba
unsigned int f03 = 3; //diferencial t1-t2 para desligar a bomba


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

// o estado serial bom usar um esquema de bits, onde cada bit setado seria um tipo de erro
// em decimal seria a somatoria que daria um resultado interpretavel
int estado = 0; //estado geral: 0 sem erros, 1 erro de sensor, outros erros
int p = 0;

float t1, t2; // temperaturas dos sensores t1-placa t2 piscina

TimerM timer1;
TimerM tmr_config;
TimerM tmr_b1_1;
TimerM tmr_b1_2;

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
    Bomba1(); // muda estado da bomba1 se necessario
    Bomba1_tempo();
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

//void mostra_endereco_sensor(DeviceAddress deviceAddress)
//{
//  for (uint8_t i = 0; i < 8; i++) {
//    // Adiciona zeros se necessário
//    if (deviceAddress[i] < 16) Serial.print("0");
//    Serial.print(deviceAddress[i], HEX);
//  }
//}


void Sensor(char action) {
  if (action == 'i') { // inicializa sensores
    sensor1.begin();
    sensor2.begin();
    if (sensor1.getDeviceCount() == 1 && sensor2.getDeviceCount() == 1) {
      estado = 0;
      sensor1.getAddress(s1, 0);
      sensor2.getAddress(s2, 0);
    } else {
      estado = 1;
    }
    StatusLed();
    LogError();

  } else if (action == 'r') { // le sensores somente se estiver OK
    if (estado == 0) {
      sensor1.requestTemperatures();
      sensor2.requestTemperatures();
      t1 = sensor1.getTempC(s1);
      t2 = sensor2.getTempC(s2);
      if (t1 < -10 || t2 < -10) { // se a temperatura estiver fora da faixa e porque houve erro
        estado = 2;
      }
    }
  }
  StatusLed();
  LogError();
}

// imprime os parametros de configuracao
void Config(char action) {
  if (action == 'p') { // imprime
    Serial.print("{");
    Serial.print("v:"); Serial.print(v);
    Serial.print(",f02:"); Serial.print(f02);
    Serial.print(",f03:"); Serial.print(f03);
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
    Serial.print("e:"); Serial.print(estado);
    Serial.print(",t1:"); Serial.print(t1);
    Serial.print(",t2:"); Serial.print(t2);
    Serial.print(",b1:"); Serial.print(b1);
    Serial.print(",b2:"); Serial.print(b2);
    Serial.print(",b1_tempo:"); Serial.print(b1_tempo);
    Serial.println("}");
  }
}

void Bomba1() { // muda o estado da bomba1 se necessário
  if (estado == 0) {
    if (t1 - t2 > f02) { // liga
      b1 = 1; digitalWrite(B1_PIN, !b1); // !b1 pois o modulo de rele é invertido
    }
    else if (t1 - t2 < f03) { // desliga
      b1 = 0; digitalWrite(B1_PIN, !b1); // !b1 pois o modulo de rele é invertido
    }

  } else { // estado != 0 : desliga por problemas
    b1 = 0; digitalWrite(B1_PIN, !b1); // !b1 pois o modulo de rele é invertido
  }
//
//  if (b1 == 1 && tmr_b1_1.run(b1_1)) {
//    b1 = 0; digitalWrite(B1_PIN, !b1); // !b1 pois o modulo de rele é invertido
//    p = 1;
//  }
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
  if (estado == 0) {
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

