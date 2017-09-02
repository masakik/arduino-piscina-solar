
#include <OneWire.h>
#include <DallasTemperature.h>

// Porta do pino de sinal do DS18B20
#define ONE_WIRE_BUS 3

// Define uma instancia do oneWire para comunicacao com o sensor
OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

int v = 1;

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

  } else if (action == 'l') { // carrega

  }
}

void Sensor(char action) {
  if (action == 'i') { // inicializa sensores
    sensors.begin();
    estado = (sensors.getDeviceCount() != 2) ? 1 : 0;
    sensors.getAddress(s1, 0);
    sensors.getAddress(s2, 1);
  } else if (action == 'r') { // le sensores
    sensors.requestTemperatures();
    t1 = sensors.getTempC(s1);
    t2 = sensors.getTempC(s2);
  }
}

void Estados(char action) {
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

void Bomba1() { // muda o estado da bomba 1 se necessário
  if (t1 - t2 > f02) {
    b1 = 1;
  } else if (t1 - t2 < f03) {
    b1 = 0;
  }
}

bool Timer1(long int intervalo) {
  if (millis() - tmr1 > intervalo) { //roda o timer
    tmr1 = millis();
    return true;
  } else {
    return false;
  }
}

bool Timer2(long int intervalo) {
  if (millis() - tmr2 > intervalo) { //roda o timer
    tmr2 = millis();
    return true;
  } else {
    return false;
  }
}

void loop()
{
  if (Timer1(5000)) {
    Sensor('i'); // inicializa
    Sensor('r'); // le
    Bomba1(); // muda estado da bomba1
    Estados('p'); // Mostra dados no serial monitor
  }

  if (Timer2(15000)) {
    Sensor('i');
    Config('p');
  }


  //delay(5000);
}
