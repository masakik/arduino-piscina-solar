
#include <OneWire.h>
#include <DallasTemperature.h>

// Porta do pino de sinal do DS18B20
#define ONE_WIRE_BUS 3
#define B1_PIN 4 //pino do rele da bomba1
#define B2_PIN 5 //pino do rele da bomba2

#define LEDR 11
#define LEDG 12
#define LEDB 13

// Define uma instancia do oneWire para comunicacao com o sensor
OneWire oneWire(ONE_WIRE_BUS);

// Indica onde procurar os sensores de temperatura
DallasTemperature sensors(&oneWire);

int v = 1; //versao

// parametros ajustaveis
int f02 = 8;  //diferencial t1-t2 para ligar a bomba
int f03 = 4; //diferencial t1-t2 para desligar a bomba
DeviceAddress s1, s2; // s1 end do sensor da placa, s2 end do sensor da piscina

// parametros internos
bool b1 = 0; //bomba da placa de aquecimento
bool b2 = 0; //bomba do filtro

// o estado serial bom usar um esquema de bits, onde cada bit setado seria um tipo de erro
// em decimal seria a somatoria que daria um resultado interpretavel
int estado = 0; //estado geral: 0 sem erros, 1 erro de sensor, outros erros

float t1, t2; // temperaturas dos sensores

long int tmr1 = 0, tmr2 = 0; //temporizadores

void setup(void)
{
  Serial.begin(9600);
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
  StatusLed();

  Sensor('i');
  Config('p');
}

void mostra_endereco_sensor(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++) {
    // Adiciona zeros se necessário
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

// imprime os parametros de configuracao
void Config(char action) {
  if (action == 'p') { // imprime
    Serial.print("{");
    Serial.print("v:");
    Serial.print(v);
    Serial.print(",s1:");
    mostra_endereco_sensor(s1);
    Serial.print(",s2:");
    mostra_endereco_sensor(s2);
    Serial.print(",f02:");
    Serial.print(f02);
    Serial.print(",f03:");
    Serial.print(f03);
    Serial.println("}");
  } else if (action == 's') { // salva
    // salva
  } else if (action == 'l') { // carrega
    // carrega
  } else if (action == 'r') { // reseta a configuracao para o padrao
    // reset
  }
}

void Sensor(char action) {
  if (action == 'i') { // inicializa sensores
    sensors.begin();
    if (sensors.getDeviceCount() == 2) {
      estado = 0;
      sensors.getAddress(s1, 0);
      sensors.getAddress(s2, 1);
    } else if (sensors.getDeviceCount() == 1) {
      estado = 1;
      sensors.getAddress(s1, 0);
    } else if (sensors.getDeviceCount() == 0) {
      estado = 2;
    }
    StatusLed();
    LogError();

  } else if (action == 'r') { // le sensores somente se estiver OK
    sensors.requestTemperatures();
    t1 = sensors.getTempC(s1);
    t2 = sensors.getTempC(s2);

//    if (t1 < -10) { // se a temperatura estver fora da faixa e porque houve erro
//      estado = 3;
//    }
//    if (t2 < -10) {
//      estado = 4;
//    }

  }
}

void Variaveis(char action) {
  if (action == 'p') { // imprime dados
    Serial.print("{");
    Serial.print("estado:");
    Serial.print(estado);
    Serial.print(",t1:");
    Serial.print(t1);
    Serial.print(",t2:");
    Serial.print(t2);
    Serial.print(",b1:");
    Serial.print(b1);
    Serial.print(",b2:");
    Serial.print(b2);
    Serial.println("}");
  }
}

void Bomba1() { // muda o estado da bomba1 se necessário
  if (estado == 0) {
    if (t1 - t2 > f02) {
      b1 = 1;
      digitalWrite(B1_PIN, !b1);
    } else if (t1 - t2 < f03) {
      b1 = 0;
      digitalWrite(B1_PIN, !b1);
    }
  } else {
    b1 = 0;
    digitalWrite(B1_PIN, !b1);
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

bool Timer1(unsigned int intervalo) {
  if (millis() - tmr1 > intervalo * 1000) { //roda o timer
    tmr1 = millis();
    return true;
  } else {
    return false;
  }
}

bool Timer2(unsigned int intervalo) {
  if (millis() - tmr2 > intervalo * 1000) { //roda o timer
    tmr2 = millis();
    return true;
  } else {
    return false;
  }
}

void loop()
{

  if (Timer1(5)) {
    Sensor('i'); // inicializa
    Sensor('r'); // le
    Bomba1(); // muda estado da bomba1 se necessario
    Variaveis('p'); // Mostra dados no serial monitor
  }

  if (Timer2(60)) {
    Sensor('i');
    Config('p'); // mostra configuracao
  }

}
