# arduino piscina solar
Controle de aquecedor solar para piscina.
O objetivo desse sistema é implementar o controle da bomba de circulação de água nas placas solares a fim
de aquecer a piscina de casa.

Opera em modo diferencial com um sensor na placa de aquecimento e outro dentro da piscina. Dessa forma é minimizado o tempo de aquecimento da piscina. 

Ao atingir a temperatura desejada o sistema impede o funcionamento da bomba.

Possui sistema anticongelamento que faz a bomba circular água nas placas se sua temperatura ficar muito baixa.

## Módulos
* Arduino
* ESP8266
* DS18B20
* Módulo RTC
* Módulo de relê
* fonte
* caixa
* leds de sinalizacao


## Comunicacao WEB
* setar parametros / ler parametros / configuracao padrao
* ler dados :  temperatura t1, temperatura t2, estado da bomba, tempo acumulado ligado diario, alarme de congelamento das placas, 

## Parâmetros configuráveis

Sensor 1: Sensor da placa de aquecimento

Sensor 2: Sensor da piscina

[F1] - Temperatura desejável da piscina

[F2] – Diferencial (T1-T2) para ligar a bomba. Padrao: 8

[F3] – Diferencial (T1-T2) para desligar a bomba. Padrao: 4

[F4] – Temperatura de anticongelamento (T1) para ligar a bomba:
Quando a temperatura dos coletores (T1) estiver muito baixa (ex.: noites de inverno) a bomba é
ligada, em função da temperatura ajustada neste parâmetro, para impedir que a água congele
no coletor solar e danifique o mesmo. A bomba ficará ligada até o tempo F5 qunado testará 
novamente se a temperatura da placa subiu acima de F4, repetindo o ciclo se necessário.
Padrao: 8

[F5]– Tempo mínimo de anticongelamento:
Este tempo mínimo de bomba ligada quando atinge a temperatura de anticongelamento.
Mesmo que a temperatura do sensor T1 supere a temperatura de anticongelamento,
o controlador respeita o tempo programado neste parâmetro. 
padrao: 40 (segundos)

## Sinalização

Erro (e): Monitora o estado dos sensores e ativa quando há problemas.
* 0: Funcionamento normal
* 1: Erro no sensor 1;
* 2: Erro no sensor 2;

Info (i): Informa a operação em andamento. 
* 0: bomba1 desligada; 
* 1: bomba ligada no modo de aquecimento diferencial (operação normal)
* 2: a piscina atingiu a temperatura desejada (F1);
* 4: a bomba está ligada por atingir a temperatura de anticongelamento (F4)


