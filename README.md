# arduino piscina solar
Controle de aquecedor solar para piscina.
O objetivo desse sistema é implementar o controle da bomba de circulação de água nas placas solares a fim
de aquecer a piscina de casa.

## Módulos
* Arduino
* ESP8266
* DS18B20
* Módulo RTC
* Módulo de relê

## Comunicacao WEB
* setar parametros / ler parametros
* ler dados :  temperatura t1/t2, tempo acumulado ligado diario, alarme de congelamento das placas, 

## Parâmetros configuráveis
[F02]– Diferencial (T1-T2) para ligar a bomba:
Permite configurar a diferença de temperatura entre o coletor solar e a piscina para acionar a
bomba de circulação. Amedida que os coletores recebem energia, a temperatura no sensor T1
aumenta, quando esta temperatura estiver a um determinado valor acima da temperatura do
sensor T2, a bomba é ligada e circula a água aquecida, armazenando-a na piscina.
Padrao: 8

[F03]– Diferencial (T1-T2) para desligar a bomba:
Permite configurar a diferença de temperatura entre o coletor solar e a piscina para desligar a
bomba de circulação. Com a bomba ligada, a diferença de temperatura entre o coletor e a
piscina (T1-T2) tende a diminuir. Quando este valor cai a um determinado valor, a bomba é
desligada, parando a circulação da água.
Padrao: 4

[F04]– Temperatura de anticongelamento (T1) para ligar a bomba:
Quando a temperatura dos coletores (T1) estiver muito baixa (ex.: noites de inverno) a bomba é
ligada, em função da temperatura ajustada neste parâmetro, para impedir que a água congele
no coletor solar e danifique o mesmo. Ahisterese deste controle é fixa e definida em 2.0ºC. Para
desabilitar esta função desloque o ajuste para o mínimo até que seja exibido[,,No].
Padrao: 8

[F05]– Tempo mínimo de anticongelamento:
Este tempo mínimo de bomba ligada, serve como segurança, para garantir que a água passe
por todos os coletores. Mesmo que a temperatura do sensor T1 supere a temperatura de
anticongelamento (parâmetro [,F04]), o controlador respeita o tempo programado neste
parâmetro. Função muito utilizada em grandes em obras pela quantidade de placas instaladas.
Para desabilitar esta função desloque o ajuste para o mínimo até que seja exibido [,,No].
padrao: 60


